#include "EventLoop.h"
#include <sys/eventfd.h>
#include <unistd.h>
#include <iostream>
#include <sys/syscall.h>


int EventLoop::MkEventfd() {
    int efd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if (efd < 0) {
        perror("eventfd creation failed");
        abort();
    }
    return efd;
}

EventLoop::EventLoop()
    : _is_looping(false),
      _is_stop(false),
      _is_event_handleing(false),
      _is_handling_pending_functions(false),
      _poller(std::make_shared<Mypoll>()),
      _event_fd(MkEventfd()),
      _wakeup_channel(std::make_shared<Channel>(_event_fd)),
      thread_id(static_cast<pid_t>(::syscall(SYS_gettid))) { 

    _wakeup_channel->SetEvents(EPOLLIN | EPOLLET);
    // _wakeup_channel->SetReadHandler([this]() {
    //     HandleRead(_event_fd);
    // });
    _wakeup_channel->SetReadHandler(std::bind(&EventLoop::HandleRead, this));

    _wakeup_channel->SetUpdateHandler(std::bind(&EventLoop::HandleUpdate, this));
    PollerAdd(_wakeup_channel);
}

EventLoop::~EventLoop() {
    close(_event_fd);
}

void EventLoop::Loop() {
    if (_is_looping) {
        std::cerr << "[ERROR] EventLoop is already looping at address: " << this << std::endl;
        abort();  // Exit immediately to avoid undefined behavior
    }

    _is_looping = true;
    _is_stop = false;

    std::cerr << "[INFO] Starting EventLoop at address: " << this << std::endl;

    while (!_is_stop) {
        std::vector<std::shared_ptr<Channel>> readyChannels;
        if (!_poller) {
            std::cerr << "[ERROR] Poller is not initialized in EventLoop!" << std::endl;
            abort();
        }
        _poller->poll(readyChannels);

        _is_event_handleing = true;
        for (auto& channel : readyChannels) {
            if (!channel) {
                std::cerr << "[ERROR] Null channel encountered!" << std::endl;
                continue;
            }
            channel->HandleEvents();
        }
        _is_event_handleing = false;

        HandlePendingFunctions();
    }

    _is_looping = false;
    std::cerr << "[INFO] Stopping EventLoop at address: " << this << std::endl;
}

void EventLoop::HandleExpiredTimers() {
    // Check for expired timers and execute their callbacks
    // TimerQueue is assumed to be integrated into EventLoop
    auto expiredTimers = TimerQueue::getInstance().getExpiredTimers();

    for (const auto& timer : expiredTimers) {
        timer->executeCallback(); // Execute the associated callback
    }
}

void EventLoop::StopLoop() {
    _is_stop = true;
    AsynWake(); // Wake up the loop to stop
}

void EventLoop::RunInLoop(Func&& func) {
    if (IsInLoopThread()) {
        func();
    } else {
        QueueLoop(std::move(func));
    }
}

void EventLoop::QueueLoop(Func&& func) {
    {
        MutexLockGuard lock(mutex_);
        pendingFunction.emplace_back(std::move(func));
    }
    AsynWake(); // Wake up the loop to handle pending functions
}

bool EventLoop::IsInLoopThread() const {
    return thread_id == static_cast<pid_t>(::syscall(SYS_gettid)); // Compare thread IDs
}

void EventLoop::HandleUpdate() {
    PollerMod(_wakeup_channel); // Update the wakeup channel in the poller
}

void EventLoop::HandleRead() {
    if (_event_fd > 0) {
        uint64_t one = 1;
        ssize_t n = read(_event_fd, &one, sizeof(one));
        if (n != sizeof(one)) {
            perror("[ERROR] HandleRead: failed to read from eventfd");
        } else {
            std::cerr << "[INFO] HandleRead: eventfd read successful, value: " << one << std::endl;
        }
    } else {
        std::cerr << "[ERROR] HandleRead: invalid event_fd" << std::endl;
    }
}


void EventLoop::HandlePendingFunctions() {
    std::vector<Func> funcs;
    _is_handling_pending_functions = true;

    {
        MutexLockGuard lock(mutex_);
        funcs.swap(pendingFunction); // Take all pending functions
    }

    for (auto& func : funcs) {
        try {
            func();
        } catch (const std::exception& e) {
            std::cerr << "Error executing pending function: " << e.what() << std::endl;
        }
    }
    _is_handling_pending_functions = false;
}

// void EventLoop::HandleRead(int fd) {
//     char buffer[1024];
//     while (true) {
//         ssize_t n = read(fd, buffer, sizeof(buffer));

//         if (n > 0) {
//             std::string data(buffer, n);
//             std::cerr << "[INFO] Received: " << data << " from fd " << fd << std::endl;

//             // Optionally, respond to the client
//             std::string response = "Echo: " + data;
//             HandleWrite(fd, response);
//         } else if (n == 0) {
//             std::cerr << "[INFO] Connection closed by client, fd: " << fd << std::endl;
//             PollerDel(std::make_shared<Channel>(fd));
//             close(fd);
//             break;
//         } else {
//             if (errno == EAGAIN || errno == EWOULDBLOCK) {
//                 // Re-register the interest in reading
//                 auto channel = std::make_shared<Channel>(fd);
//                 channel->SetEvents(EPOLLIN | EPOLLONESHOT);
//                 channel->SetReadHandler([this, fd]() { HandleRead(fd); });
//                 PollerMod(channel);
//                 break;
//             } else {
//                 perror("[ERROR] HandleRead: read failed");
//                 PollerDel(std::make_shared<Channel>(fd));
//                 close(fd);
//                 break;
//             }
//         }
//     }
// }


void EventLoop::HandleWrite(int fd, const std::string& response) {
    ssize_t n = write(fd, response.c_str(), response.size());
    if (n > 0) {
        std::cerr << "[INFO] HandleWrite: Sent response to fd " << fd << ": " << response << std::endl;
    } else if (n == 0) {
        std::cerr << "[INFO] HandleWrite: No data written to fd " << fd << std::endl;
    } else {
        perror("[ERROR] HandleWrite: write failed");
    }
}

void EventLoop::AsynWake() {
    uint64_t one = 1;
    ssize_t n = write(_event_fd, &one, sizeof(one));
    if (n != sizeof(one)) {
        perror("AsynWake error: failed to write to eventfd");
    }
}

void EventLoop::Shutdown(std::shared_ptr<Channel> channel) {
    if (channel) {
        int fd = channel->Getfd();
        shutDownWR(fd);  // shut down the write end of the socket
        PollerDel(channel);
    }
}

void EventLoop::PollerAdd(std::shared_ptr<Channel> channel, int timeout) {
    if (_poller) {
        _poller->epoll_add(channel);
    }
}

void EventLoop::PollerMod(std::shared_ptr<Channel> channel, int timeout) {
    if (_poller) {
        _poller->epoll_mod(channel);
    }
}

void EventLoop::PollerDel(std::shared_ptr<Channel> channel) {
    if (_poller) {
        _poller->epoll_del(channel);
    }
}