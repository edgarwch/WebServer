#include <iostream>
#include <string.h>
#include "Thread.h"
#include "CurrentThread.h"


Thread::Thread(const ThreadFunc& func, const std::string& name):
        started_(false),
        joined_(false),
        pthreadId_(0),
        tid_(0),
        func_(func),
        name_(name),
        latch_(1)
{
    if(name_.empty()){
        setDefaultName();
    }   
}


Thread::~Thread(){
    if(started_ && !joined_){
        pthread_detach(pthreadId_);// Detach the thread if not joined
    }
}

// Give a default name if I'm lazy to give one
void Thread::setDefaultName(){
    if(name_.empty()){
        name_ = "Thread";
    }
}

void Thread::start() {
    started_ = true;

    auto startRoutine = [](void* arg) -> void* {
        Thread* thread = static_cast<Thread*>(arg);
        try {
            CurrentThread::cacheTid();
            thread->tid_ = CurrentThread::tid();
            CurrentThread::t_threadName = thread->name_.c_str();
            thread->latch_.countDown();
            thread->func_();
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] Exception in thread: " << e.what() << std::endl;
            throw;
        } catch (...) {
            std::cerr << "[ERROR] Unknown exception in thread." << std::endl;
            throw;
        }
        return nullptr;
    };

    if (pthread_create(&pthreadId_, nullptr, startRoutine, this) != 0) {
        started_ = false;
        perror("[ERROR] pthread_create failed");
    } else {
        latch_.wait();
    }
}


int Thread::join() {
    if (started_ && !joined_) {
        joined_ = true;
        int ret = pthread_join(pthreadId_, nullptr);
        if (ret != 0) {
            std::cerr << "[ERROR] pthread_join failed: " << strerror(ret) << std::endl;
        } else {
            std::cerr << "[INFO] Successfully joined thread " << name_ << std::endl;
        }
        return ret;
    }
    std::cerr << "[WARNING] Thread already joined or not started." << std::endl;
    return -1;
}