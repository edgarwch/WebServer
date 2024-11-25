#pragma once

#include <functional>
#include <sys/epoll.h>
#include <memory>

class HttpConnection;

class Channel {
public:
    typedef std::function<void()> Callback;

    Channel();
    explicit Channel(int fd);  // ban implicit conversion !
    ~Channel();

    void HandleEvents();
    void HandleWrite();
    void HandleRead();
    void HandleUpdate();
    void HandleError();

    int Getfd() const { return _fd; }
    void Setfd(int fd) { _fd = fd; }

    std::shared_ptr<HttpConnection> GetHolder() const { return _holder.lock(); }
    void SetHolder(std::shared_ptr<HttpConnection> holder) { _holder = holder; }

    void SetReadHandler(Callback&& readHandler) { _read_handler = readHandler; }
    void SetWriteHandler(Callback&& writeHandler) { _write_handler = writeHandler; }
    void SetUpdateHandler(Callback&& updateHandler) { _update_handler = updateHandler; }
    void SetErrorHandler(Callback&& errorHandler) { _error_handler = errorHandler; }

    void SetRevents(int revents) { _revents = revents; }
    int& events() { return _events; }
    void SetEvents(int events) { _events = events; }
    int GetLastEvents() const { return _last_events; }
    void UpdateLastEvents() { _last_events = _events; }

private:
    int _fd;
    int _events;
    int _revents;
    int _last_events;

    Callback _read_handler;
    Callback _write_handler;
    Callback _update_handler;
    Callback _error_handler;

    std::weak_ptr<HttpConnection> _holder;
};

typedef std::shared_ptr<Channel> spChannel;
