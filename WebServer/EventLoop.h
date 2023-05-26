#pragma once
#include <functional>
class EventLoop{
public:
    typedef std::function<void()> Func;

    EventLoop();
    ~EventLoop();
    
    void Loop();
    void StopLoop();

    void RunInLoop(Func&& func);

    void QueueLoop(Func&& func);

    //polleradd

    //poller mod

    //poller del

    //shutdown!


private:

    int event_fd_;

    pid_t thread_id;

    bool is_stop_;
    bool is_looping_;
    bool is_event_handleing_;
    bool is_handling_pending_functions_;

    static int MkEventfd();
    void HandleUpdate();
    void HandleRead();
    void AsynWake();
    void HandlePendingFUnctions();



    std::vector<Func> pendingFunction;


};