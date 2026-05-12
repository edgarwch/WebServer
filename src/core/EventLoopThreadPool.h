#pragma once

#include "EventLoop.h"
#include <memory>
#include <vector>
#include <thread>
#include <atomic>

class EventLoopThreadPool {
public:
    explicit EventLoopThreadPool(int numThreads);
    ~EventLoopThreadPool();

    EventLoopThreadPool(const EventLoopThreadPool&) = delete;
    EventLoopThreadPool& operator=(const EventLoopThreadPool&) = delete;

    void start();
    void stop();

    EventLoop* nextLoop();
    int size() const noexcept { return numThreads_; }

private:
    struct Worker {
        std::unique_ptr<EventLoop> loop;
        std::jthread thread;
    };

    int numThreads_;
    std::atomic<size_t> nextIndex_{0};
    std::vector<std::unique_ptr<Worker>> workers_;
};
