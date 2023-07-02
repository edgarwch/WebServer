#pragma once
#include <memory>
#include <vector>
#include "EventLoopThread.h"
#include "../utils/noncopyable.h"
typedef std::shared_ptr<EventLoop> sp_EventLoop;

class EventLoopThreadPool:noncopyable{
    public:
        EventLoopThreadPool(int numThreads);
        ~EventLoopThreadPool(){};
        void start();
        sp_EventLoop getNextLoop();
    
    private:
        int _numThread;
        int _index;
        std::vector<std::shared_ptr<EventLoopThread>> _threads;
};