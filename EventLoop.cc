#include "EventLoop.h"
#include "Logger.h"
#include "Poller.h"
#include "Channel.h"

#include <sys/eventfd.h>
#include <errno.h>
#include <unistd.h>


//防止一个线程创建多个EventLoop
__thread EventLoop *t_loopInThisThread = nullptr;

//定义默认的Poller IO多路复用接口的超时时间
const int kPollTimeMs = 10000;

//调用eventfd函数创建eventfd，进行跨线程唤醒
int createEventfd() {
    int eventFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (eventfd < 0) {
        LOG_FATAL("eventfd error:%d \n", errno);
    }
    return eventFd;
}

//构造函数
EventLoop::EventLoop()
    : looping_(false)
    , quit_(false)
    , threadId_(CurrentThread::tid())
    , poller_(Poller::newDefaultPoller(this))
    , wakeupFd_(createEventfd())
    , wakeupChannel_(new Channel(this, wakeupFd_))
{
    LOG_DEBUG("EventLoop create %p in thread %d \n", this, threadId_);
    if (t_loopInThisThread) 
    {
        LOG_FATAL("Another EventLoop exists in this thread %d \n", t_loopInThisThread, threadId_);
    }
    else 
    {
        t_loopInThisThread = this;
    }

    //设置wakeupFd_的事件类型以及发生事件后的回调操作
    wakeupChannel_->setReadCallback(
        [this](Timestamp)
        {
            this->handleRead();
        });
    //每一个eventLoop都将监听wakeupFd_的EPOLLIN读事件了
    wakeupChannel_->enableReading();
}

EventLoop::~EventLoop() {
    //不关注任何事件，并更新在epoll中的状态(从epoll关注列表中删除)
    wakeupChannel_->disableAll();
    //从Poll的ChannelList中删除
    wakeupChannel_->remove();
    ::close(wakeupFd_);
    // 已经将该线程唯一EventLoop析构，该线程不存在EventLoop了，再创建就不会LOG_FATAL了
    t_loopInThisThread = nullptr;
}

//开启事件循环
void EventLoop::loop() {
    looping_ = true;
    quit_ = false;
    LOG_INFO("EventLoop %p start looping \n", this);
    
    while (!quit_) {
        activeChannels_.clear();
        //监听两类fd  一种是client的，一类是wakeupfd
        pollReturnTime_ = poller_->Poll(kPollTimeMs, &activeChannels_);
        for (Channel * channel : activeChannels_) {
            channel->handleEvent(pollReturnTime_);
        }
        //执行当前EventLoop需要执行的回调操作
        doPendingFunctors();
    }
    LOG_INFO("EventLoop %p stop looping", this);
    looping_ = false;
}

//执行回调
void EventLoop::doPendingFunctors() {
    std::vector<Functor> functors;
    callingPendingFunctors_ = true;

    //将列表中的待执行的回调与空的列表交换，直接从定义的局部变量functors中取回调执行，让其他线程能直接往回调列表中写数据
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for (const Functor& functor : functors) {
        functor();
    }
    callingPendingFunctors_ = false;
}

//退出事件循环  1.loop在自己的线程中调用quit  2.在非loop的线程中，调用loop的quit
void EventLoop::quit() {
    quit_ = true;

    //如果是在其他线程调用的quit，还需要唤醒对应线程
    if (!isInLoopThread()) {
        wakeup();
    }
}

//wakeupFd_注册的读回调事件，将事件读出，用来防止？
void EventLoop::handleRead()
{
    uint64_t one = 1;
    ssize_t n = ::read(wakeupFd_, &one, sizeof(one));

    if (n != sizeof(one)) {
        LOG_ERROR("EventLoop::handleRead() reads %d bytes instead of 8", n);
    }
}

//用来跨线程唤醒对应线程的EventLoop，往eventfd写数据唤醒epoll监听的eventfd的读事件
void EventLoop::wakeup()
{
    uint64_t one = 1;
    ssize_t n = ::write(wakeupFd_, &one, sizeof(one));

    if (n != sizeof(one)) {
        LOG_ERROR("EventLoop::wakeup() writes %d bytes instead of 8", n);
    }
}

//在当前EventLoop中执行cb
void EventLoop::runInLoop(Functor cb) {
    if (isInLoopThread()) {  //在当前loop线程中，执行cb
        cb();
    }
    else {  //在非当前loop线程中执行，就需要唤醒loop所在线程，执行cb
        queueInLoop(cb);
    }
}

//把cd放入队列中，唤醒loop所在线程执行cb
void EventLoop::queueInLoop(Functor cb) {
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.emplace_back(std::move(cb));
    }
    //callingPendingFunctors_为true说明当前loop正在执行doPendingFunctors()回调，为了防止
    if (!isInLoopThread() || callingPendingFunctors_) {
        wakeup();    //唤醒loop所在线程
    }
}

 //EventLoop的方法=》调用Poller的成员方法
void EventLoop::updateChannel(Channel *channel) {
    poller_->updateChannel(channel);
}

void EventLoop::removeChannel(Channel *channel)
{
    poller_->removeChannel(channel);
}

bool EventLoop::hasChannel(Channel *channel) {
    return poller_->hasChannel(channel);
}