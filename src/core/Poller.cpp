#include "Poller.h"
#include "../util/Logger.h"
#include <unistd.h>

Poller::Poller()
    : epollfd_(::epoll_create1(EPOLL_CLOEXEC))
    , events_(kInitEventVecSize)
{
    if (!epollfd_) {
        Logger::error() << "epoll_create1 failed: " << errno;
        std::abort();
    }
}

void Poller::add(const ChannelPtr& channel) {
    int fd = channel->fd();
    epoll_event ev;
    ev.events = channel->events();
    ev.data.fd = fd;

    fd2chan_[fd] = channel;
    channel->updateLastEvents();

    if (::epoll_ctl(epollfd_.get(), EPOLL_CTL_ADD, fd, &ev) < 0) {
        Logger::error() << "epoll_ctl ADD fd=" << fd << " failed: " << errno;
        fd2chan_.erase(fd);
    }
}

void Poller::update(const ChannelPtr& channel) {
    int fd = channel->fd();
    epoll_event ev;
    ev.events = channel->events();
    ev.data.fd = fd;

    // Only issue EPOLL_CTL_MOD if events actually changed
    if (channel->lastEvents() != channel->events()) {
        if (::epoll_ctl(epollfd_.get(), EPOLL_CTL_MOD, fd, &ev) < 0) {
            Logger::error() << "epoll_ctl MOD fd=" << fd << " failed: " << errno;
            fd2chan_.erase(fd);
        }
    }
    channel->updateLastEvents();
}

void Poller::remove(const ChannelPtr& channel) {
    int fd = channel->fd();
    epoll_event ev{};
    // EPOLL_CTL_DEL requires a non-null pointer on older kernels
    ::epoll_ctl(epollfd_.get(), EPOLL_CTL_DEL, fd, &ev);
    fd2chan_.erase(fd);
}

std::vector<Poller::ChannelPtr> Poller::poll(int timeoutMs) {
    int n = ::epoll_wait(epollfd_.get(), events_.data(),
                         static_cast<int>(events_.size()), timeoutMs);
    if (n < 0) {
        if (errno == EINTR) return {};
        Logger::error() << "epoll_wait failed: " << errno;
        return {};
    }

    std::vector<Poller::ChannelPtr> ready;
    ready.reserve(static_cast<size_t>(n));
    for (int i = 0; i < n; ++i) {
        int fd = events_[static_cast<size_t>(i)].data.fd;
        auto it = fd2chan_.find(fd);
        if (it == fd2chan_.end()) continue;
        it->second->setRevents(events_[static_cast<size_t>(i)].events);
        ready.push_back(it->second);
    }
    return ready;
}
