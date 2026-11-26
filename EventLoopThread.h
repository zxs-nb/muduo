#pragma once
#include "noncopyable.h"
#include "Thread.h"

#include <functional>
#include <mutex>
#include <condition_variable>


class EventLoop;

class EventLoopThread : noncopyable
{
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    EventLoopThread(const ThreadInitCallback &cb = ThreadInitCallback(), const std::string &name = std::string());
    ~EventLoopThread();

    //需要在启动时把地址传给上层线程池
    EventLoop *startLoop();

private:
    void threadFunc();

    EventLoop *loop_; //获取子线程创建在栈上的EventLoop的地址
    Thread thread_;
    std::condition_variable cond_;//用于传给线程对象的绑定函数中实现EventLoop创建的同步
    std::mutex mutex_;
    ThreadInitCallback callback_; //提供给上层用户的扩展接口
};