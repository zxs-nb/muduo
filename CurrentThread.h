#pragma once

namespace CurrentThread
{
    //__thread关键字让该全局变量变成每一个线程都能创建的一份副本，记录当前执行代码的线程id
    extern __thread int t_cachedTid;//用于缓存执行当前代码的线程id，以免每次都进行系统调用

    void cacheTid();

    inline int tid()
    {
        //告诉内核t_cachedTid == 0大概率为0，并返回t_cachedTid == 0的结果，
        //即t_cachedTid大概率不为0，依次帮助内核优化xxx,
        if (__builtin_expect(t_cachedTid == 0, 0)) {
            cacheTid();  //如果t_cachedTid为0，调用cacheTid获取当前执行代码的线程id
        }
        return t_cachedTid;
    }
}