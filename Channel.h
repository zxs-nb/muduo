#pragma conce
#include "noncopyable.h"
#include "Timestamp.h"

#include <functional>
#include <memory>

class EventLoop;


//Channel 理解为通道，封装了sockfd和其感兴趣的event，如EPOLLIN、EPOLLOUT事件
//还绑定了poller返回的具体事件
class Channel : noncopyable
{
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    // fd得到poller通知以后，处理事件的
    void handleEvent(Timestamp receiveTime);

    //设置回调函数对象
    void setReadCallback(ReadEventCallback cb) { readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb) { writeCallback_ = std::move(cb); }
    void setCloseCallback(EventCallback cb) { closeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb) { errorCallback_ = std::move(cb); }

    //防止EventLoop还在调用Channel的回调函数时，poller已经把Channel绑定的fd清除，并remove了Channel
    void tie(const std::shared_ptr<void>&);

    //提供外界访问的接口，在Poller中更新事件，存储map值时会用到
    int fd() const { return fd_; }
    int events() const { return events_; }
    void setRevents(int revt) { revents_ = revt; }

    // 设置fd相应的事件状态
    void enableReading() { events_ |= kReadEvent;
        update();
    }
    void disableReading() { events_ &= ~kReadEvent;
        update();
    }
    void enableWriting() { events_ |= kWriteEvent;
        update();
    }
    void disableWriting() { events_ &= ~kWriteEvent;
        update();
    }
    void disableAll() { events_ &= kNoneEvent;
        update();
    }

    //返回fd相应的事件状态
    bool isNoneEvent() const { return events_ == kNoneEvent; }
    bool isReadEvent() const { return events_ == kReadEvent; }
    bool isWriteEvent() const { return events_ == kWriteEvent; }

    //设置index_
    void setIndex(int index) { index_ = index; }
    int index() const { return index_; }

    EventLoop* ownerLoop() const { return loop_; }
    
    //处理从拥有该Channel的EventLoop中删除该Channel
    void remove();

private:
    void update(); //向上层EventLoop通知更新该fd感兴趣的事件

    void handleEventWithGuard(Timestamp receiveTime);

    static const int kNoneEvent; //表示不关注任何IO事件
    static const int kReadEvent; //关注普通可读和高优先级可读事件
    static const int kWriteEvent; //关注可写事件

    EventLoop* loop_; //事件循环
    const int fd_; // fd, poller监听的对象
    int events_;  // 注册fd感兴趣的事件
    int revents_;  //接收poller实际返回的发生事件
    int index_;   //用来表示当前Channel是否已被注册到epoll中

    std::weak_ptr<void> tie_;
    bool tied_;   

    EventCallback writeCallback_; 
    EventCallback closeCallback_; 
    EventCallback errorCallback_; 
    ReadEventCallback readCallback_; 
};