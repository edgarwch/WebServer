#include "HttpConnection.h"
#include "SocketOps.h"
#include "../util/Logger.h"
#include <unistd.h>
#include <sys/socket.h>

HttpConnection::HttpConnection(UniqueFd fd, EventLoop* loop)
    : owner_(loop)
    , fd_(std::move(fd))
    , channel_(std::make_shared<Channel>(fd_.get()))
{
    setNonBlocking(fd_.get());
}

HttpConnection::~HttpConnection() = default;

void HttpConnection::start() {
    channel_->setEvents(EPOLLIN | EPOLLET);

    auto self = weak_from_this();
    channel_->onRead([self] {
        if (auto conn = self.lock()) conn->handleRead();
    });
    channel_->onWrite([self] {
        if (auto conn = self.lock()) conn->handleWrite();
    });
    channel_->onError([self] {
        if (auto conn = self.lock()) conn->handleError();
    });

    // Register with the event loop; connection stays alive via shared_ptr
    owner_->addChannel(channel_);
}

void HttpConnection::handleRead() {
    // Edge-triggered: keep reading until EAGAIN
    while (true) {
        ssize_t n = ::read(fd_.get(), readBuf_.data(), readBuf_.size());
        if (n > 0) {
            std::string_view data(readBuf_.data(), static_cast<size_t>(n));
            if (context_.parse(data)) {
                // Complete request parsed — build and write response
                writeBuf_ = buildResponse();
                writeOffset_ = 0;

                // Switch to write monitoring
                channel_->setEvents(EPOLLOUT | EPOLLET);
                owner_->updateChannel(channel_);

                // Start writing
                handleWrite();
                return;
            }
        } else if (n == 0) {
            Logger::info() << "Client disconnected fd=" << fd_.get();
            close();
            return;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // No more data — wait for next edge trigger
                break;
            }
            Logger::error() << "Read error fd=" << fd_.get() << ": " << errno;
            close();
            return;
        }
    }
}

void HttpConnection::handleWrite() {
    while (writeOffset_ < writeBuf_.size()) {
        ssize_t n = ::write(fd_.get(),
                            writeBuf_.data() + writeOffset_,
                            writeBuf_.size() - writeOffset_);
        if (n > 0) {
            writeOffset_ += static_cast<size_t>(n);
        } else if (n == 0) {
            break;
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Kernel buffer full — wait for next EPOLLOUT
                break;
            }
            Logger::error() << "Write error fd=" << fd_.get() << ": " << errno;
            close();
            return;
        }
    }

    if (writeOffset_ >= writeBuf_.size()) {
        // All data written — close (Connection: close)
        Logger::info() << "Response sent to fd=" << fd_.get();
        close();
    }
}

void HttpConnection::handleError() {
    Logger::error() << "Error on fd=" << fd_.get();
    close();
}

void HttpConnection::close() {
    if (channel_) {
        owner_->removeChannel(channel_);
        channel_.reset();
    }
    // UniqueFd will close the fd on destruction
}

std::string HttpConnection::buildResponse() {
    const auto& req = context_.request();

    std::string body;
    body += "<!DOCTYPE html>\n<html>\n<body>\n";
    body += "<h1>WebServer</h1>\n";
    body += "<p>Method: " + req.method + "</p>\n";
    body += "<p>Path: " + req.path + "</p>\n";
    body += "<p>Version: " + req.version + "</p>\n";

    if (!req.headers.empty()) {
        body += "<h2>Headers</h2>\n<ul>\n";
        for (const auto& [k, v] : req.headers) {
            body += "<li>" + k + ": " + v + "</li>\n";
        }
        body += "</ul>\n";
    }

    if (!req.body.empty()) {
        body += "<h2>Body</h2>\n<pre>" + req.body + "</pre>\n";
    }

    body += "</body>\n</html>\n";

    std::string resp;
    resp += "HTTP/1.1 200 OK\r\n";
    resp += "Content-Type: text/html; charset=utf-8\r\n";
    resp += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    resp += "Connection: close\r\n";
    resp += "\r\n";
    resp += body;
    return resp;
}
