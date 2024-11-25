#pragma once
#include "EventLoop.h"
#include "MemoryPool.h"
#include <iostream>
#include <thread>
class EventLoopThread {
public:
    EventLoopThread() : 
        _loop(std::make_shared<EventLoop>()), 
        _thread(nullptr) {}

    ~EventLoopThread() {
        if (_thread && _thread->joinable()) {
            _thread->join();
        }
    }

    void start() {
        _thread = std::make_unique<std::thread>(&EventLoopThread::threadFunction, this);
        std::cerr << "[INFO] Thread started with EventLoop: " << _loop.get() << std::endl;
    }

    std::shared_ptr<EventLoop> getLoop() { return _loop; }

private:
    void threadFunction() {
        if (!_loop) {
            std::cerr << "[ERROR] EventLoop is nullptr in thread!" << std::endl;
            return;
        }
        _loop->Loop();
    }

    std::shared_ptr<EventLoop> _loop;
    std::unique_ptr<std::thread> _thread;
};
