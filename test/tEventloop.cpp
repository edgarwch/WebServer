#include "tEventloop.h"
__thread EventLoop* _t_loopInThisThread = 0;

EventLoop::EventLoop():
_looping(false),
_threadId(std::this_thread::get_id())
{
    std::cout<< "EventLoop created " << this << " in thread " << _threadId << std::endl;
    if(_t_loopInThisThread){
        std::cout<<" Another EventLoop " << _t_loopInThisThread << " exists in this thread " << _threadId << std::endl;
    }
    else{
        _t_loopInThisThread = this;
    }
}

EventLoop::~EventLoop(){
    assert(!_looping);
    _t_loopInThisThread = NULL;
}

void EventLoop::loop(){
    assert(!_looping);
    AssertInLoopThread();
    _looping = true;
    sleep(5);
    std::cout<< "EventLoop "<< this << " stop looping" << std::endl;
    _looping = false;
}

void EventLoop::abortNotInLoopThread(){
    std::cout<< "EventLoop::abortNotInLoopThread - EventLoop " << this << " was created in threadId_ = " << _threadId << ", current thread id = " << std::this_thread::get_id() << std::endl;
    abort();
}