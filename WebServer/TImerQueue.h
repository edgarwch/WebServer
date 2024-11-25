#include "../utils/noncopyable.h"
#include <queue>
#include <vector>
#include <functional>
#include <chrono>
#include <mutex>
#include <memory>

class Timer {
public:
    using TimePoint = std::chrono::steady_clock::time_point;
    using Callback = std::function<void()>;

    Timer(TimePoint expiry, Callback cb)
        : expiryTime(expiry), callback(std::move(cb)) {}

    TimePoint getExpiry() const { return expiryTime; }
    void executeCallback() { callback(); }

    bool operator<(const Timer& other) const {
        return expiryTime > other.expiryTime;
    }

private:
    TimePoint expiryTime;
    Callback callback;
};

class TimerQueue:noncopyable {
public:
    static TimerQueue& getInstance() {
        static TimerQueue instance;
        return instance;
    }

    void addTimer(const Timer::TimePoint& time, Timer::Callback cb) {
        std::lock_guard<std::mutex> lock(queueMutex);
        timerQueue.emplace(time, std::move(cb));
    }

    std::vector<std::shared_ptr<Timer>> getExpiredTimers() {
        std::vector<std::shared_ptr<Timer>> expired;
        auto now = std::chrono::steady_clock::now();

        std::lock_guard<std::mutex> lock(queueMutex);
        while (!timerQueue.empty() && timerQueue.top().getExpiry() <= now) {
            expired.push_back(std::make_shared<Timer>(timerQueue.top()));
            timerQueue.pop();
        }

        return expired;
    }

private:
    TimerQueue() = default;

    std::priority_queue<Timer> timerQueue;
    std::mutex queueMutex;
};
