#pragma once
#include "EventLoopThreadPool.h"
#include "SocketUtils.h"
#include <iostream>
#include <string>
#include <memory>

class Server {
public:
    Server(int port, int numThreads);
    ~Server() = default;

    void start();

private:
    void handleNewConnection(int listenFd);
    void handleClient(int connFd);
    void handleClient(int connFd, sp_EventLoop loop);

    int _port;
    int _listenFd;
    std::shared_ptr<EventLoopThreadPool> _threadPool;
};
