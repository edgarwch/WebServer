#pragma once
#include <memory>
#include <vector>
#include <mutex>
#include "EventLoopThread.h"
#include "../utils/noncopyable.h"

typedef std::shared_ptr<EventLoop> sp_EventLoop;

class EventLoopThreadPool : noncopyable {
public:
    explicit EventLoopThreadPool(int numThreads);
    ~EventLoopThreadPool();

    void start();                  // Start all threads
    sp_EventLoop getNextLoop();    // Get the next EventLoop in round-robin fashion

private:
    int _numThreads;               // Number of threads in the pool
    int _index;                    // Index for round-robin
    std::vector<std::shared_ptr<EventLoopThread>> _threads; // Pool of threads
    std::mutex _mutex;             // Mutex for thread safety
};
