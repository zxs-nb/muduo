#pragma once
#include "noncopyable.h"
#include "Timestamp.h"

#include <unordered_map>
#include <vector>

class EventLoop;
class Channel;
// muduo库中多路事件分发器的核心IO复用模块
class Poller : noncopyable
{
public:
    using ChannelList = std::vector<Channel*>;

    Poller(EventLoop* loop);
    virtual ~Poller() = default;
    virtual Timestamp Poll(int timeoutMs, ChannelList* activeChannels) = 0;

    //根据Channel更新的关注事件，调用epoll_ctl更新epoll关注的事件
    virtual void updateChannel(Channel*) = 0;
    //根据Channel更新的状态，调用epoll_ctl从epoll关注的事件中将对应的socket删除
    virtual void removeChannel(Channel*) = 0;

    //判断是否拥有某个Channel
    bool hasChannel(Channel *) const;

    //Eventloop可以从该接口获取默认实现的IO多路复用的实现
    static Poller* newDefaultPoller(EventLoop* loop);

protected:
    //Channel中保存的sockfd作为键，Channel指针作为值
    using ChannelMap = std::unordered_map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop* onwerLoop_; //定义Poller所属的事件循环的EventLoop
};