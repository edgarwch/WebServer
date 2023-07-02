#include "EventLoop.h"
#include "MemoryPool.h"
#include <thread>
class EventLoopThread{
    public:
        EventLoopThread() : _loop(NewElement<EventLoop>(), DeleteElement<EventLoop>), _thread(NewElement<std::thread>(std::bind(&EventLoopThread::threadFunction, this), DeleteElement<std::thread>)) {
        }
        ~EventLoopThread();
        void start(){ 
            _thread = std::make_unique<std::thread>(&EventLoopThread::threadFunction, this);
        }
        typedef std::shared_ptr<EventLoop> sp_EventLoop;
        sp_EventLoop getLoop(){ return _loop;}
    private:
        void threadFunction(){
            _loop->Loop();
        };
        sp_EventLoop _loop;
        std::unique_ptr<std::thread, std::function<void(std::thread*)>> _thread;
};