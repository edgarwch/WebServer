#pragma once

#include "../core/Channel.h"
#include "../core/EventLoop.h"
#include "../util/UniqueFd.h"
#include "HttpContext.h"
#include <array>
#include <string>
#include <memory>

class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
public:
    HttpConnection(UniqueFd fd, EventLoop* loop);
    ~HttpConnection();

    HttpConnection(const HttpConnection&) = delete;
    HttpConnection& operator=(const HttpConnection&) = delete;

    void start();

private:
    void handleRead();
    void handleWrite();
    void handleError();
    void close();

    std::string buildResponse();

    EventLoop* owner_;
    UniqueFd fd_;
    std::shared_ptr<Channel> channel_;
    HttpContext context_;

    // Read buffer
    static constexpr size_t kReadBufSize = 4096;
    std::array<char, kReadBufSize> readBuf_{};

    // Write buffer
    std::string writeBuf_;
    size_t writeOffset_ = 0;
};
