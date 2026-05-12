#include "EventLoopThreadPool.h"
#include "../util/Logger.h"

EventLoopThreadPool::EventLoopThreadPool(int numThreads)
    : numThreads_(numThreads)
{
    workers_.reserve(static_cast<size_t>(numThreads));
    for (int i = 0; i < numThreads; ++i) {
        workers_.push_back(std::make_unique<Worker>());
        workers_.back()->loop = std::make_unique<EventLoop>();
    }
}

EventLoopThreadPool::~EventLoopThreadPool() {
    stop();
}

void EventLoopThreadPool::start() {
    for (int i = 0; i < numThreads_; ++i) {
        auto& worker = workers_[static_cast<size_t>(i)];
        EventLoop* loop = worker->loop.get();
        worker->thread = std::jthread([loop](std::stop_token st) {
            loop->loop(st);
        });
        Logger::info() << "Started worker " << i << " with EventLoop " << loop;
    }
}

void EventLoopThreadPool::stop() {
    // jthread destructor will request_stop + join
    for (auto& w : workers_) {
        if (w->thread.joinable()) {
            w->thread.request_stop();
        }
    }
    for (auto& w : workers_) {
        if (w->thread.joinable()) {
            w->thread.join();
        }
    }
}

EventLoop* EventLoopThreadPool::nextLoop() {
    // Lock-free round-robin
    size_t idx = nextIndex_.fetch_add(1, std::memory_order_relaxed) % static_cast<size_t>(numThreads_);
    return workers_[idx]->loop.get();
}
