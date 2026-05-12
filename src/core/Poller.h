#pragma once

#include "Channel.h"
#include "../util/UniqueFd.h"
#include <vector>
#include <unordered_map>
#include <sys/epoll.h>

class Poller {
public:
    using ChannelPtr = std::shared_ptr<Channel>;

    Poller();
    ~Poller() = default;

    Poller(const Poller&) = delete;
    Poller& operator=(const Poller&) = delete;

    void add(const ChannelPtr& channel);
    void update(const ChannelPtr& channel);
    void remove(const ChannelPtr& channel);
    std::vector<ChannelPtr> poll(int timeoutMs = 1000);

private:
    static constexpr int kInitEventVecSize = 1024;

    UniqueFd epollfd_;
    std::vector<epoll_event> events_;
    std::unordered_map<int, ChannelPtr> fd2chan_;
};
