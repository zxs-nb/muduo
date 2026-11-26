#include "EventLoopThread.h"
#include "EventLoop.h"

EventLoopThread::EventLoopThread(const ThreadInitCallback &cb, const std::string &name)
    : loop_(nullptr)
    , thread_(std::bind(&EventLoopThread::threadFunc, this), name)
    , callback_(cb)
{}

EventLoopThread::~EventLoopThread() 
{
    //析构时将事件循环给退出,随后传给线程对象的执行函数threadFunc()也退出
    //栈上的EventLoop对象析构，将线程对象管理的线程一并回收
    if (loop_ != nullptr) {
        loop_->quit();
        thread_.join();
    }
}

//需要在启动时把地址传给上层线程池
EventLoop *EventLoopThread::startLoop() {
    thread_.start();  //启动底层的线程
    EventLoop *loop = nullptr;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock, [&]() {
            return loop_ != nullptr;
        });
        loop = loop_;
    }
    return loop;
}

//传给新线程的执行函数
void EventLoopThread::threadFunc() {
    EventLoop loop; //创建一个独立的eventloop，和上面的线程一一对应
    //上层应用传入了初始化回调函数，就执行，未传入则执行后续逻辑
    if (callback_) {
        callback_(&loop);
    }
    //线程间同步，必须保证子线程已经创建好了EventLoop
    //并且给上层对象EventLoopThread的成员变量loop_赋值
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }

    loop.loop(); //开始执行事件循环，EventLoop调用底层Poller的Poll开始等待事件发生

    std::unique_lock<std::mutex> lock(mutex_);
    loop_ = nullptr;
}