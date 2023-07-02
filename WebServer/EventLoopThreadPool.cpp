#include "EventLoopThreadPool.h"

EventLoopThreadPool::EventLoopThreadPool(int numThreads):_numThread(numThreads), _index(0){
    _threads.reserve(_numThread);
    for(int i = 0; i < _numThread; ++i){
        std::shared_ptr<EventLoopThread> t(NewElement<EventLoopThread>(), DeleteElement<EventLoopThread>);
        _threads.emplace_back(t);
    }
}

void EventLoopThreadPool::start(){
    for(auto& thread : _threads){
        thread->start();
    }
}

sp_EventLoop EventLoopThreadPool::getNextLoop(){
    int index = (index + 1)%_numThread;
    return _threads[index]->getLoop();
}