#include "EventLoopThreadPool.h"
#include <iostream>

EventLoopThreadPool::EventLoopThreadPool(int numThreads)
    : _numThreads(numThreads), _index(0) {
    for (int i = 0; i < _numThreads; ++i) {
        auto thread = std::make_shared<EventLoopThread>();
        _threads.emplace_back(thread);
    }
}

EventLoopThreadPool::~EventLoopThreadPool() {
    for (auto& thread : _threads) {
        if (auto loop = thread->getLoop()) {
            loop->StopLoop(); // Ensure the event loop stops
        }
    }
}

void EventLoopThreadPool::start() {
    for (int i = 0; i < _numThreads; ++i) {
        if (!_threads[i]) {
            std::cerr << "[ERROR] Thread " << i << " is null!" << std::endl;
            continue;
        }
        _threads[i]->start();
        std::cerr << "[INFO] Started thread " << i << " with EventLoop: " << _threads[i]->getLoop().get() << std::endl;
    }
}

sp_EventLoop EventLoopThreadPool::getNextLoop() {
    if (_threads.empty()) {
        std::cerr << "[ERROR] No threads in the thread pool!" << std::endl;
        return nullptr;
    }

    auto loop = _threads[_index]->getLoop();
    if (!loop) {
        std::cerr << "[ERROR] Failed to retrieve EventLoop for thread index: " << _index << std::endl;
    } else {
        std::cerr << "[INFO] Returning EventLoop at address: " << loop.get() << " for thread index: " << _index << std::endl;
    }

    _index = (_index + 1) % _numThreads;
    return loop;
}
