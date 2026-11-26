#include "Thread.h"
#include "CurrentThread.h"

#include <semaphore.h>

std::atomic_int32_t numCreated_ = 0;

Thread::Thread(ThreadFunc func, const std::string &name)
    : started_(false)
    , joined_(false)
    , tid_(0)
    , name_(name)
    , func_(std::move(func))
{
    setDefaultName();
}

Thread::~Thread() {
    if (started_ && !joined_) {
        thread_->detach();  // thread类提供的设置线程分离的方法
    }
}

void Thread::setDefaultName() {
    int num = ++numCreated_;
    if (name_.empty()) {
        char buf[64] = {0};
        snprintf(buf, sizeof(buf), "Thread%d", num);
        name_ = buf;
    }
}

void Thread::start() {  //一个Thread对象，记录的是一个新线程的详细信息
    started_ = true;
    sem_t sem;
    sem_init(&sem, 0, 0);
    //开启线程
    thread_ = std::make_unique<std::thread>([&]() {
        //获取线程id的值
        tid_ = CurrentThread::tid();
        //已经执行完了，增加一个信号量
        sem_post(&sem);
        //开启一个新线程，专门执行该线程函数
        func_();
    });
    //阻塞等待新线程id的值获取完
    sem_wait(&sem);
}

void Thread::join() {
    if (joined_)
        return;
    joined_ = true;
    thread_->join();
}
