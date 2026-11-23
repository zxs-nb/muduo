#include "Poller.h"
#include "Channel.h"

Poller::Poller(EventLoop* loop) 
    : onwerLoop_(loop)
{
}

//判断是否拥有某个Channel
bool Poller::hasChannel(Channel* channel) const {
    auto it = channels_.find(channel->fd());
    return it != channels_.end() && it->second == channel;
}


