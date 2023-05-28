#include "EventLoop.h"
#include <sys/eventfd.h>
__thread EventLoop* loopInThisThread = 0;
int EventLoop::MkEventfd(){
    int efd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(efd < 0){
        abort();
    }
    return efd;
}
EventLoop::EventLoop(): 
_is_looping(false), _is_stop(false),
_is_event_handleing(false), _is_handling_pending_functions(false),
_poller(new Mypoll()),
_event_fd(MkEventfd()),
_wakeup_channel(new Channel(_event_fd))
{
    _wakeup_channel->SetEvents(EPOLLIN | EPOLLET);
    _wakeup_channel->SetReadHandler(std::bind(&EventLoop::HandleRead, this));
    //Connection and etc..
    _wakeup_channel->SetUpdateHandler(std::bind(&EventLoop::HandleUpdate, this));
}

EventLoop::~EventLoop(){
    close(_event_fd);
    loopInThisThread = NULL;
}

void EventLoop::Loop(){
    
    if(_is_looping){
        abort();
    }
    if(!IsInLoopThread()){
        abort();
    }
    _is_looping = true;
    _is_stop = false;
    while(!_is_stop){
        std::vector<spChannel>readyChannels;
        _poller->poll(readyChannels);
        _is_event_handleing = true;
        for(auto channel: readyChannels){
            channel->HandleEvents();
        }
        _is_event_handleing = false;
        // handle pendings
        HandlePendingFUnctions();
        // TODO handle expired
    }
    _is_looping = false;
}

void EventLoop::RunInLoop(Func &&func){
    if(IsInLoopThread){
        func();
        return;
    }
    QueueLoop(std::move(func));
}

void EventLoop::QueueLoop(Func &&func){
    //lock
    pendingFunction.emplace_back(func);
    if(!IsInLoopThread() || _is_handling_pending_functions) AsynWake();
}

bool EventLoop::IsInLoopThread(){
    return false;
}

void EventLoop::HandleUpdate(){
    PollerMod(_wakeup_channel);
}

void EventLoop::HandleRead(){
    // TODO read operation
    // _wakeup_channel->SetEvents(EPOLLIN | EPOLLET);
}

void EventLoop::HandlePendingFUnctions(){
    std::vector<Func> funcs;
    _is_handling_pending_functions = true;
    // lock
    // funcs.swap(pendingFunction)
    // for (size_t i = 0; i < funcs.size(); ++i) funcs[i]();
    _is_handling_pending_functions = false;
}

void EventLoop::AsynWake(){
 // TODO write
}