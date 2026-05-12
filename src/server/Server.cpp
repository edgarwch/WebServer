#include "Server.h"
#include "../net/HttpConnection.h"
#include "../net/SocketOps.h"
#include "../util/Logger.h"
#include <csignal>
#include <sys/socket.h>

namespace {
Server* g_server = nullptr;

void signalHandler(int sig) {
    if (sig == SIGINT || sig == SIGTERM) {
        Logger::info() << "Received signal " << sig << ", shutting down";
        if (g_server) g_server->shutdown();
    }
}
} // namespace

Server::Server(int port, int numThreads)
    : port_(port)
    , listenFd_(createAndListen(port))
    , threadPool_(std::make_unique<EventLoopThreadPool>(numThreads))
{
    if (!listenFd_) {
        Logger::error() << "Failed to bind and listen on port " << port;
        std::exit(EXIT_FAILURE);
    }

    ::signal(SIGPIPE, SIG_IGN);
    g_server = this;

    struct sigaction sa{};
    sa.sa_handler = signalHandler;
    sa.sa_flags = 0;
    ::sigemptyset(&sa.sa_mask);
    ::sigaction(SIGINT, &sa, nullptr);
    ::sigaction(SIGTERM, &sa, nullptr);
}

Server::~Server() {
    g_server = nullptr;
}

void Server::start() {
    threadPool_->start();
    Logger::info() << "Server listening on port " << port_;

    while (running_.load(std::memory_order_acquire)) {
        int connFd = ::accept4(listenFd_.get(), nullptr, nullptr,
                               SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (connFd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                continue;
            }
            if (errno == EINTR) {
                continue;
            }
            Logger::error() << "accept4 failed: " << errno;
            break;
        }

        EventLoop* loop = threadPool_->nextLoop();
        auto conn = std::make_shared<HttpConnection>(UniqueFd(connFd), loop);
        loop->runInLoop([conn] {
            conn->start();
        });
    }

    Logger::info() << "Server stopped";
}

void Server::shutdown() {
    running_.store(false, std::memory_order_release);
    threadPool_->stop();
}
