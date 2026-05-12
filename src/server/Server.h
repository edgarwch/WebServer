#pragma once

#include "../core/EventLoopThreadPool.h"
#include "../net/SocketOps.h"
#include "../util/UniqueFd.h"
#include <atomic>

class Server {
public:
    Server(int port, int numThreads);
    ~Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    void start();
    void shutdown();

private:
    void acceptLoop();

    int port_;
    UniqueFd listenFd_;
    std::unique_ptr<EventLoopThreadPool> threadPool_;
    std::atomic<bool> running_{true};
};
