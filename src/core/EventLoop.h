#pragma once

#include "Poller.h"
#include "TimerQueue.h"
#include "../util/UniqueFd.h"
#include <functional>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <stop_token>

class EventLoop {
public:
    using Func = std::function<void()>;

    EventLoop();
    ~EventLoop();

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    void loop(std::stop_token st);
    void stop();

    void runInLoop(Func fn);
    void queueInLoop(Func fn);

    void addChannel(std::shared_ptr<Channel> ch);
    void updateChannel(std::shared_ptr<Channel> ch);
    void removeChannel(std::shared_ptr<Channel> ch);

    bool isInLoopThread() const noexcept;
    void assertInLoopThread() const;

    TimerQueue& timerQueue() noexcept { return timerQueue_; }

private:
    static UniqueFd createEventfd();
    void wakeup();
    void handleWakeup();
    void runPendingFunctions();
    void handleExpiredTimers();

    UniqueFd eventfd_;
    std::thread::id ownerId_;

    std::atomic<bool> stop_{false};
    bool looping_ = false;

    std::unique_ptr<Poller> poller_;
    std::shared_ptr<Channel> wakeupChannel_;
    TimerQueue timerQueue_;

    std::mutex pendingMutex_;
    std::vector<Func> pendingFunctions_;
};
