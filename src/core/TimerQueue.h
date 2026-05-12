#pragma once

#include <queue>
#include <vector>
#include <functional>
#include <chrono>
#include <mutex>
#include <memory>

struct Timer {
    using TimePoint = std::chrono::steady_clock::time_point;
    using Callback = std::function<void()>;

    Timer(TimePoint expiry, Callback cb)
        : expiryTime(expiry), callback(std::move(cb)) {}

    TimePoint expiryTime;
    Callback callback;

    // Min-heap: smaller expiry = higher priority
    bool operator<(const Timer& other) const {
        return expiryTime > other.expiryTime;
    }
};

class TimerQueue {
public:
    TimerQueue() = default;

    TimerQueue(const TimerQueue&) = delete;
    TimerQueue& operator=(const TimerQueue&) = delete;

    void add(Timer::TimePoint when, Timer::Callback cb) {
        std::lock_guard lock(mutex_);
        queue_.emplace(when, std::move(cb));
    }

    std::vector<Timer> expired() {
        std::vector<Timer> result;
        auto now = std::chrono::steady_clock::now();

        std::lock_guard lock(mutex_);
        while (!queue_.empty() && queue_.top().expiryTime <= now) {
            result.push_back(queue_.top());
            queue_.pop();
        }
        return result;
    }

private:
    std::priority_queue<Timer> queue_;
    std::mutex mutex_;
};
