#pragma once
#include <iostream>
#include <stdint.h>
#include <unistd.h>
#include <sys/syscall.h>

namespace CurrentThread {
extern thread_local int t_cachedTid;
extern thread_local const char* t_threadName;

void cacheTid();

inline int tid() {
    if (__builtin_expect(t_cachedTid == 0, 0)) {
        cacheTid();
    }
    return t_cachedTid;
}

inline const char* name() {
    std::cerr << "[INFO] Thread name: " << t_threadName << std::endl;
    return t_threadName;
}

}
