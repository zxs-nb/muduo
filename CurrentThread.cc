#include "CurrentThread.h"
#include <syscall.h>
#include <unistd.h>

namespace CurrentThread
{
    __thread int t_cachedTid = 0;

    void cacheTid() {
        if (t_cachedTid == 0) {
            // 通过系统调用获取当前线程在 Linux 内核中的真实 ID（LWP）
            t_cachedTid = static_cast<pid_t>(::syscall(SYS_gettid));
        }
    }
}