#pragma once
#include <functional>
#include <vector>
#include "Channel.h"
#include "Mypoll.h"
#include "../utils/util.h"
#include "SocketUtils.h"
class EventLoop{
    public:
        // callback function
        typedef std::function<void()> Func;

        EventLoop();
        ~EventLoop();
        
        void Loop();
        void StopLoop();

        void RunInLoop(Func&& func);

        void QueueLoop(Func&& func);

        //polleradd
        void PollerAdd(std::shared_ptr<Channel> channel, int timeout = 0){
            _poller->epoll_add(channel);
        };
        //poller mod
        void PollerMod(std::shared_ptr<Channel> channel, int timeout = 0){
            _poller->epoll_mod(channel);
        };
        //poller del
        void PollerDel(std::shared_ptr<Channel> channel){
            _poller->epoll_del(channel);
        };
        //shutdown!
        void Shutdown(std::shared_ptr<Channel> channel){
            // SOCKET shutdown
            // clear the buffer first
        };

        bool IsInLoopThread();

    private:

        int _event_fd;

        pid_t thread_id;

        bool _is_stop;
        bool _is_looping;
        bool _is_event_handleing;
        bool _is_handling_pending_functions;

        static int MkEventfd();
        void HandleUpdate();
        void HandleRead(); // by read from event_fd
        void AsynWake(); // by write to event_fd
        void HandlePendingFUnctions();

        std::shared_ptr<Channel> _wakeup_channel;

        std::shared_ptr<Mypoll> _poller;

        // mutex lock

        std::vector<Func> pendingFunction;
};