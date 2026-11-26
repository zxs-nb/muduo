#pragma once
#include "noncopyable.h"
#include "Timestamp.h"
#include "CurrentThread.h"

#include <vector>
#include <functional>
#include <atomic>
#include <memory>
#include <mutex>

class Channel;
class Poller;
// 事件循环类 主要包含了两个大模块 Channel  Poller(epoll的抽象)
class EventLoop : noncopyable
{
public:
    using Functor = std::function<void()>;

    EventLoop();
    ~EventLoop();

    //开启事件循环
    void loop();
    //退出事件循环
    void quit();

    //返回Poller返回事件的时间
    Timestamp pollReturnTime() const { return pollReturnTime_; }

    //在当前EventLoop中执行cb
    void runInLoop(Functor cb);
    //把cd放入队列中，唤醒loop所在线程执行cb
    void queueInLoop(Functor cb);

    //唤醒对应subLoop所属的线程
    void wakeup();

    //EventLoop的方法=》调用Poller的成员方法
    void updateChannel(Channel *channel);
    void removeChannel(Channel *channel);
    bool hasChannel(Channel *channel);

    // 判断当前执行线程是否是该EventLoop所属的线程
    bool isInLoopThread() { return threadId_ == CurrentThread::tid(); }
private:
    void handleRead();  // 执行 wakeup事件发生的回调
    void doPendingFunctors(); //执行回调
    using ChannelList = std::vector<Channel *>;

    std::atomic_bool looping_; //标识是否在处理事件循环，用CAS实现
    std::atomic_bool quit_; //标识是否退出事件循环

    const pid_t threadId_; //记录当前EventLoop所在线程的Id

    Timestamp pollReturnTime_; //记录Poller返回事件的channels的时间点
    std::unique_ptr<Poller> poller_; //IO复用类

    int wakeupFd_; //当mainLoop获取一个新用户的channel，通过轮询算法选择一个subLoop，通过该成员变量唤醒subLoop的处理线程
    std::unique_ptr<Channel> wakeupChannel_; //绑定wakeupFd文件描述符的事件

    ChannelList activeChannels_; //接收Poller返回的活跃的Channel事件的数组

    std::atomic_bool callingPendingFunctors_; //标识当前EventLoop是否还有需要执行的回调操作
    std::vector<Functor> pendingFunctors_; //存储EventLoop需要执行的所有回调操作
    std::mutex mutex_; //用来保护上面的vector容器的线程安全操作
};