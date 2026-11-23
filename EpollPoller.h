#pragma once
#include "Timestamp.h"

#include <sys/epoll.h>
#include "Poller.h"
#include <vector>

class EpollPoller : public Poller
{
public:
    EpollPoller(EventLoop* loop);
    ~EpollPoller() override;

    //重写基类的抽象方法
    Timestamp Poll(int timeoutMs, ChannelList* activeChannels) override;
    void updateChannel(Channel* channel) override;//epll_ctl
    void removeChannel(Channel* channel) override;
private:
    static const int kInitEventListSize = 16;

    //填写活跃的连接
    void fillActiveChannels(int numEvents, ChannelList* activeChannels) const;
    //更新Channel通道
    void update(int operation, Channel* channel);

    using EventList = std::vector<epoll_event>;

    EventList events_;
    int epollfd_;
};