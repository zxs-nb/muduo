#include "EpollPoller.h"
#include "Logger.h"
#include "Channel.h"

#include <errno.h>
#include <unistd.h>

const int kNew = -1;  //Channel的成员index_，表示Channel未添加到Poller中
const int kAdded = 1; //表示已添加到
const int kDeleted = 2; //表示已从epoll的监听事件中删除,但还在channels_中

EpollPoller::EpollPoller(EventLoop* loop)
    : Poller(loop)
    , epollfd_(::epoll_create1(EPOLL_CLOEXEC))
    , events_(kInitEventListSize)
{
    if (epollfd_ < 0) {
        LOG_FATAL("epoll_create error:%d \n", errno);
    }
}

EpollPoller::~EpollPoller() { ::close(epollfd_); }

//重写基类的抽象方法，EventLoop会把自己保存的vector<Channel*>传入Poll
Timestamp EpollPoller::Poll(int timeoutMs, ChannelList* activeChannels) {
    LOG_INFO("func=%s => fd total count:%zu\n", __FUNCTION__, channels_.size());

    int numEvents = ::epoll_wait(epollfd_, events_.data(), static_cast<int>(events_.size()), timeoutMs);
    int saveErrno = errno;
    Timestamp now(Timestamp::now());
    if (numEvents > 0) {
        LOG_INFO("%d events happened \n", numEvents);
        fillActiveChannels(numEvents, activeChannels);
        if (numEvents == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    }
    else if (numEvents == 0) {
        LOG_DEBUG("%s timeout!\n", __FUNCTION__);
    }
    else {
        if (saveErrno != EINTR) {
            errno = saveErrno;
            LOG_ERROR("EpollPoller Poll() error!");
        }
    }
    return now;
}

//Channel更新关注的事件后向上层EventLoop报告后调用的更新函数
void EpollPoller::updateChannel(Channel* channel) 
{
    const int index = channel->index();
    LOG_INFO("func=%s => fd=%d events=%d index=%d \n", __FUNCTION__, channel->fd(), channel->events(), index);

    if (index == kNew || index == kDeleted) {
        if (index == kNew)
        {
            int fd = channel->fd();
            channels_[fd] = channel;
        }
        channel->setIndex(kAdded);
        update(EPOLL_CTL_ADD, channel);
    }
    else  //index显示为已添加，说明是要修改epoll监听事件或者删除
    {
        if (channel->isNoneEvent()) {  //什么事件都不关注，说明是要删除
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(kDeleted);
        }
        else {
            update(EPOLL_CTL_MOD, channel);          
        }
    }
}

//当不关注任何事件了，即events==kNoneEvent
void EpollPoller::removeChannel(Channel* channel)
{
    LOG_INFO("func=%s => fd=%d\n", __FUNCTION__, channel->fd());
    int fd = channel->fd();
    channels_.erase(fd);

    int index = channel->index();
    if (index == kAdded) 
    {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->setIndex(kNew);
}

//填写活跃的连接，回填给EventLoop，让EventLoop来处理对应的回调
void EpollPoller::fillActiveChannels(int numEvents, ChannelList* activeChannels) const {
    for (int i = 0; i < numEvents; ++i) {
        Channel *channel = static_cast<Channel *>(events_[i].data.ptr);
        channel->setRevents(events_[i].events);
        activeChannels->push_back(channel);//EventLoop就拿到了它的Poller给它返回的所有发生事件的channel列表了       
    }
}


//更新Channel通道，epoll_ctl add/mod/del
void EpollPoller::update(int operation, Channel* channel) {
    epoll_event event{};
    int fd = channel->fd();
    event.events = channel->events();
    event.data.ptr = channel;
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) 
    {
        if (operation == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl del error:%d\n", errno);
        }
        else{
            LOG_FATAL("epoll_ctl add/mod error:%d\n", errno);
        }
    }
}