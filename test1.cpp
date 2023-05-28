#include "test/tEventloop.h"
EventLoop* g_loop;
void threadFunc(){
    std::cout<< "threadFunc(): pid = "<< getpid() << " tid = "<<  std::this_thread::get_id()<< std::endl;
    EventLoop loop;
    loop.loop();
}

int main(){
    std::cout<< "threadFunc(): pid = "<< getpid() << " tid = "<<  std::this_thread::get_id() << std::endl;
    
    EventLoop loop;
    std::thread td(threadFunc);
    loop.loop();
    pthread_exit(NULL);
    return 0;
}