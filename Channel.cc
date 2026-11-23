#include "Channel.h"
#include "EventLoop.h"
#include "Logger.h"

#include <sys/epoll.h>

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::kWriteEvent = EPOLLOUT;


//one loop per pthread
Channel::Channel(EventLoop* Loop, int fd)
    : loop_(Loop)
    , fd_(fd)
    , events_(0)
    , revents_(0)
    , index_(-1)
    , tied_(false)
{
}

Channel::~Channel() {}

//绑定持有Channel的TcpConnection，观察是否发生析构，
//因为业务处理最终是TcpConnection注册给Channel，
//让Channel最终作为中间体与EventLoop进行交互，调用回调，真正执行函数
void Channel::tie(const std::shared_ptr<void>& obj) {
    tie_ = obj;
    tied_ = true;
}

//当改变Channel所绑定的fd所表示的events事件后，
//update负责在Poller里面更改fd相应的事件(epoll_ctl)
void Channel::update() {
    // 通过Channel所属的EventLoop,调用poller的相应方法，注册fd的events事件
    // loop_->updateChannel(this);
}

//处理从拥有该Channel的EventLoop中删除该Channel
void Channel::remove() {
    // lope_->removeChannel(this);
}

// fd得到poller通知以后，处理事件的
void Channel::handleEvent(Timestamp receiveTime) {
    if (tied_) {
        std::shared_ptr<void> guard = tie_.lock();
        if (guard) {
            handleEventWithGuard(receiveTime);
        }
        //如果为空，说明绑定过对应的TcpConnection，但已经析构，故不做任何处理
    }
    //如果没被绑定过，说明是不需要绑定TcpConnection的Channel
    else {
        handleEventWithGuard(receiveTime);
    }
}

//真正的回调处理函数
void Channel::handleEventWithGuard(Timestamp receiveTime) {
    LOG_INFO("Channel handle events: %d\n", revents_);
    //关闭
    if (revents_ & EPOLLHUP && !(revents_ & EPOLLIN)) {
        if (closeCallback_) {   //对端发生关闭，epoll触发EPOLLHUP，同时缓冲区已经没有可读事件了
            closeCallback_();
        }
    }
    //错误
    if (revents_ & EPOLLERR) {
        if (errorCallback_) {
            errorCallback_();
        }
    }
    //写
    if (revents_ & (EPOLLIN | EPOLLPRI)) {
        if (readCallback_) {
            readCallback_(receiveTime);
        }
    }
    //读
    if (revents_ & EPOLLOUT) {
        if (writeCallback_) {
            writeCallback_();
        }
    }
}