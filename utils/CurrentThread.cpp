#include "CurrentThread.h"
namespace CurrentThread {
thread_local int t_cachedTid = 0;              
thread_local const char* t_threadName = "Main";

void cacheTid() {
    if (t_cachedTid == 0) {
        t_cachedTid = static_cast<int>(::syscall(SYS_gettid));
        if (t_cachedTid == -1) {
            perror("[ERROR] syscall SYS_gettid failed");
            abort();
        }
        std::cerr << "[INFO] Cached thread ID: " << t_cachedTid << std::endl;
    }
}
}