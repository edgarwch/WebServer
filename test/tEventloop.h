#pragma once
#include "../utils/noncopyable.h"
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <thread>
#include <iostream>
#include <assert.h>

class EventLoop:noncopyable{
    public:
        EventLoop();
        ~EventLoop();
        void loop();
        bool IsINLoopThread() const{
            return _threadId == std::this_thread::get_id();
        }
        void AssertInLoopThread(){
            if(!IsINLoopThread()){
                abortNotInLoopThread();
            }
        }
    private:
        void abortNotInLoopThread();
        bool _looping;
        const std::thread::id _threadId;
};