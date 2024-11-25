#pragma once
#include <functional>
#include <vector>
#include <memory>
#include "Channel.h"
#include "Mypoll.h"
#include "TImerQueue.h"
#include "../utils/util.h"
#include "SocketUtils.h"
#include "../utils/MutexLock.h"

class EventLoop {
public:
    using Func = std::function<void()>;

    EventLoop();
    ~EventLoop();

    void Loop();        // Start
    void StopLoop();    // Stop

    void RunInLoop(Func&& func);    // Execute
    void QueueLoop(Func&& func);   // Queue

    void PollerAdd(std::shared_ptr<Channel> channel, int timeout = 0);
    void PollerMod(std::shared_ptr<Channel> channel, int timeout = 0);
    void PollerDel(std::shared_ptr<Channel> channel);
    bool IsInLoopThread() const; 
    void Shutdown(std::shared_ptr<Channel> channel); // shutdown
    void HandleExpiredTimers();
    // void HandleRead(int fd);
    void HandleWrite(int fd, const std::string& response);

private:
    static int MkEventfd();        // Create eventfd
    void HandleUpdate();           // Update handler
    void HandleRead();             // Handle wakeup events
    void AsynWake();               // Wake up the loop
    void HandlePendingFunctions(); // Execute queued functions

    int _event_fd;                 // wakeup events
    bool _is_stop;
    bool _is_looping;
    bool _is_event_handleing;
    bool _is_handling_pending_functions;

    std::shared_ptr<Channel> _wakeup_channel; // Channel for wakeup events
    std::shared_ptr<Mypoll> _poller;          // Poller for events
    pid_t thread_id;
    MutexLock mutex_;
    std::vector<Func> pendingFunction; // Queued functions to execute
};
