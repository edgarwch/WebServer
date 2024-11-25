#include "Server.h"
#include <iostream>
#include <sys/socket.h>
#include <signal.h>
#include <unistd.h>

Server::Server(int port, int numThreads)
    : _port(port), 
      _listenFd(socket_bind_listen(port)),
      _threadPool(std::make_shared<EventLoopThreadPool>(numThreads)) {
    if (_listenFd < 0) {
        perror("Failed to bind and listen on the socket");
        exit(EXIT_FAILURE);
    }

    signal(SIGPIPE, SIG_IGN); // Ignore SIGPIPE to prevent crashes on broken pipes
}

void Server::start() {
    _threadPool->start();

    std::cout << "Server listening on port " << _port << "..." << std::endl;

    while (true) {
        handleNewConnection(_listenFd);
    }
}

void Server::handleNewConnection(int listenFd) {
    while (true) {
        int connFd = accept(listenFd, nullptr, nullptr);
        if (connFd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;  // No more connections to accept
            }
            perror("accept failed");
            return;
        }

        setSocketNonBlocking(connFd);

        auto loop = _threadPool->getNextLoop();
        loop->RunInLoop([this, connFd, loop]() {
            handleClient(connFd, loop);
        });
    }
}

void Server::handleClient(int connFd, sp_EventLoop loop) {
    std::cout << "New connection: " << connFd << std::endl;

    auto channel = std::make_shared<Channel>(connFd);
    channel->SetEvents(EPOLLIN | EPOLLET);
    channel->SetReadHandler([connFd]() {
        char buffer[1024];
        ssize_t n = readn(connFd, buffer, sizeof(buffer) - 1);

        if (n > 0) {
            buffer[n] = '\0'; 
            std::cout << "Received: " << buffer << " from fd " << connFd << std::endl;

            std::string response = "Echo: " + std::string(buffer);
            // writen(connFd, response.c_str(), response.size());
        } else if (n == 0) {
            std::cout << "Connection closed by client: " << connFd << std::endl;
            close(connFd);
        } 
        else {
            perror("readn error");
        }
    });

    channel->SetErrorHandler([connFd]() {
        std::cerr << "[ERROR] Error on connection fd: " << connFd << std::endl;
        close(connFd);
    });

    loop->PollerAdd(channel);
}