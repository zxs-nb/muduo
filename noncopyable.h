#pragma once

// noncopyable被继承后，派生类对象可以正常的构造和析构，但是派生类对象
// 无法进行拷贝构造和拷贝赋值

class noncopyable
{
public:
    noncopyable(const noncopyable &) = delete;
    noncopyable &operator=(const noncopyable &) = delete;
//写成protected是因为该类就是为了用来专门供其他类继承的工具基类。
//而不需要被实例化，写成protected外界就不能直接创建该对象了
//不写成private是因为protected还能保证派生类能调用该类构造析构函数，写成private派生类都无法初始化了
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};