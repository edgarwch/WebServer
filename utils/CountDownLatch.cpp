#include "CountDownLatch.h"

CountDownLatch::CountDownLatch(int count)
    : count_(count), condition_(mutex_) {
}

// lock and wait for your turn!
void CountDownLatch::wait(){
    MutexLockGuard lock(mutex_);
    while(count_ > 0){
        condition_.wait();
    }
}

// count down then notify all others
void CountDownLatch::countDown(){
    MutexLockGuard lock(mutex_);
    if(--count_ == 0){
        condition_.notifyAll();
    }
}

