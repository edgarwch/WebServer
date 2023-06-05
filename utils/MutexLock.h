#pragma once
#include <pthread.h>
#include <cstdio>
#include "noncopyable.h"

class MutexLock:noncopyable{
    public:
        MutexLock(){
            pthread_mutex_init(&_mutex, NULL);
        }
        ~MutexLock(){
            pthread_mutex_lock(&_mutex);
            pthread_mutex_destroy(&_mutex);
        }
        void Lock(){
            pthread_mutex_lock(&_mutex);
        }
        void Unlock(){
            pthread_mutex_unlock(&_mutex);
        }
        pthread_mutex_t* GetMutex(){
            return &_mutex;
        }
    private:
        pthread_mutex_t _mutex;
};


class MutexLockGard:noncopyable{
    public:
        explicit MutexLockGard(MutexLock &mutex):_mutex(mutex){ _mutex.Lock(); };
        ~MutexLockGard(){
            _mutex.Unlock();
        }
    private:
        MutexLock &_mutex;
};