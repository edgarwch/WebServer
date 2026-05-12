#include "EventLoop.h"
#include "../util/Logger.h"
#include <sys/eventfd.h>
#include <unistd.h>

UniqueFd EventLoop::createEventfd() {
    int efd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (efd < 0) {
        Logger::error() << "eventfd creation failed: " << errno;
        std::abort();
    }
    return UniqueFd(efd);
}

EventLoop::EventLoop()
    : eventfd_(createEventfd())
    , ownerId_(std::this_thread::get_id())
    , poller_(std::make_unique<Poller>())
    , wakeupChannel_(std::make_shared<Channel>(eventfd_.get()))
{
    wakeupChannel_->setEvents(EPOLLIN | EPOLLET);
    wakeupChannel_->onRead([this] { handleWakeup(); });
    poller_->add(wakeupChannel_);
}

EventLoop::~EventLoop() = default;

void EventLoop::loop(std::stop_token st) {
    if (looping_) {
        Logger::error() << "EventLoop " << this << " is already looping";
        std::abort();
    }

    // When jthread requests stop, wake us out of epoll_wait
    std::stop_callback stopCb(st, [this] { stop(); });

    looping_ = true;
    Logger::info() << "EventLoop " << this << " starting";

    while (!stop_.load(std::memory_order_acquire)) {
        auto ready = poller_->poll();

        for (auto& ch : ready) {
            ch->handleEvents();
        }

        handleExpiredTimers();
        runPendingFunctions();
    }

    looping_ = false;
    Logger::info() << "EventLoop " << this << " stopped";
}

void EventLoop::stop() {
    stop_.store(true, std::memory_order_release);
    wakeup();
}

void EventLoop::runInLoop(Func fn) {
    if (isInLoopThread()) {
        fn();
    } else {
        queueInLoop(std::move(fn));
    }
}

void EventLoop::queueInLoop(Func fn) {
    {
        std::lock_guard lock(pendingMutex_);
        pendingFunctions_.push_back(std::move(fn));
    }
    wakeup();
}

bool EventLoop::isInLoopThread() const noexcept {
    return ownerId_ == std::this_thread::get_id();
}

void EventLoop::assertInLoopThread() const {
    if (!isInLoopThread()) {
        Logger::error() << "EventLoop::assertInLoopThread failed: owner="
                        << ownerId_ << " caller=" << std::this_thread::get_id();
        std::abort();
    }
}

void EventLoop::addChannel(std::shared_ptr<Channel> ch) {
    runInLoop([this, ch = std::move(ch)] {
        poller_->add(ch);
    });
}

void EventLoop::updateChannel(std::shared_ptr<Channel> ch) {
    runInLoop([this, ch = std::move(ch)] {
        poller_->update(ch);
    });
}

void EventLoop::removeChannel(std::shared_ptr<Channel> ch) {
    runInLoop([this, ch = std::move(ch)] {
        poller_->remove(ch);
    });
}

void EventLoop::wakeup() {
    uint64_t one = 1;
    ssize_t n = ::write(eventfd_.get(), &one, sizeof(one));
    if (n != sizeof(one)) {
        Logger::error() << "EventLoop::wakeup write failed: " << errno;
    }
}

void EventLoop::handleWakeup() {
    uint64_t val = 0;
    ssize_t n = ::read(eventfd_.get(), &val, sizeof(val));
    if (n != sizeof(val)) {
        Logger::error() << "EventLoop::handleWakeup read failed: " << errno;
    }
}

void EventLoop::runPendingFunctions() {
    std::vector<Func> funcs;
    {
        std::lock_guard lock(pendingMutex_);
        funcs.swap(pendingFunctions_);
    }
    for (auto& fn : funcs) {
        fn();
    }
}

void EventLoop::handleExpiredTimers() {
    auto expired = timerQueue_.expired();
    for (auto& t : expired) {
        t.callback();
    }
}
