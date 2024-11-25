#pragma once
#include <pthread.h>
#include <cstdio>
#include "noncopyable.h"

class MutexLock:noncopyable{
    public:
        MutexLock(){
            if (pthread_mutex_init(&_mutex, nullptr) != 0) {
                perror("Failed to initialize mutex");
            }
        }
        ~MutexLock(){
            pthread_mutex_destroy(&_mutex);
        }
        void Lock(){
            pthread_mutex_lock(&_mutex);
        }
        void Unlock(){
            pthread_mutex_unlock(&_mutex);
        }
        pthread_mutex_t* get(){
            return &_mutex;
        }
    private:
        pthread_mutex_t _mutex;
};


class MutexLockGuard:noncopyable{
    public:
        explicit MutexLockGuard(MutexLock &mutex):_mutex(mutex){ _mutex.Lock(); };
        ~MutexLockGuard(){
            _mutex.Unlock();
        }
    private:
        MutexLock &_mutex;
};