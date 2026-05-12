#pragma once

#include <functional>
#include <sys/epoll.h>
#include <memory>

class Channel : public std::enable_shared_from_this<Channel> {
public:
    using Callback = std::function<void()>;

    explicit Channel(int fd) noexcept : fd_(fd) {}
    ~Channel() = default;

    Channel(const Channel&) = delete;
    Channel& operator=(const Channel&) = delete;
    Channel(Channel&&) = default;
    Channel& operator=(Channel&&) = default;

    void handleEvents();

    int fd() const noexcept { return fd_; }
    int events() const noexcept { return events_; }
    void setEvents(int ev) noexcept { events_ = ev; }
    void setRevents(int ev) noexcept { revents_ = ev; }
    int lastEvents() const noexcept { return lastEvents_; }
    void updateLastEvents() noexcept { lastEvents_ = events_; }

    void onRead(Callback cb)  { readHandler_ = std::move(cb); }
    void onWrite(Callback cb) { writeHandler_ = std::move(cb); }
    void onError(Callback cb) { errorHandler_ = std::move(cb); }

private:
    int fd_ = -1;
    int events_ = 0;
    int revents_ = 0;
    int lastEvents_ = 0;

    Callback readHandler_;
    Callback writeHandler_;
    Callback errorHandler_;
};
