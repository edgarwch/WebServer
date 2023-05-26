#pragma once
#include "Channel.h"
#include <unordered_map>
#include <stdio.h>
class Mypoll{
public:
    Mypoll();
    ~Mypoll();
    void epoll_add(const spChannel& request);
    void epoll_mod(const spChannel& request);
    void epoll_del(const spChannel& request);
    void poll(std::vector<spChannel>& request);
private:
    int _epollfd;   // epoll fd from epoll_create
    std::vector<epoll_event> _events; // epoll_wait return events
    std::unordered_map<int, spChannel> _fd2chan; // fd to channel
    int _epoll_wait_time_ms = 1000;
};
