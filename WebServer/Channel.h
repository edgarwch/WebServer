#include <functional>
#include <sys/epoll.h>
#include <memory>
class HttpConnection;
class Channel{
public:
    typedef std::function<void()> Callback;
    Channel();
    ~Channel();
    // avoid implicit conversion
    explicit Channel(int fd);
    
    void HandleEvents();
    void HandleWrite();
    void HandleRead();
    void HandleUpdate();
    void HandleError();

    int Getfd() { return _fd; };
    int Setfd(int fd) { _fd = fd; };
    
    std::shared_ptr<HttpConnection> GetHolder() { return _holder.lock(); };
    void SetHolder(std::shared_ptr<HttpConnection> holder) { _holder = holder; };

    //
    void SetReadHandler(Callback&& readHandler) { _read_handler = readHandler; };
    void SetWriteHandler(Callback&& writeHandler) { _write_handler = writeHandler; };
    void SetUpdateHandler(Callback&& updateHandler) { _update_handler = updateHandler; };
    void SetErrorHandler(Callback&& errorHandler) { _error_handler = errorHandler; };

    void SetRevents(int revents) { _revents = revents; };
    int& events() { return _events; };
    void SetEvents(int events) { _events = events; };
    int GetLastEvents() { return _last_events; };
    bool UpdateLastEvents() { _last_events = _events; };
private:
    int _fd;        // channel fd
    int _events;    // event of interest
    int _revents;   // ready events
    int _last_events;

    Callback _read_handler;
    Callback _write_handler;
    Callback _update_handler;
    Callback _error_handler;

    std::weak_ptr<HttpConnection> _holder;
};

typedef std::shared_ptr<Channel> spChannel;