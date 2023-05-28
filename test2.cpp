#include "test/tEventloop.h"
EventLoop* g_loop;
void threadFunc(){
    g_loop->loop();
}

int main(){
    
    EventLoop loop;
    g_loop = &loop;
    std::thread td(threadFunc);
    td.join();
    return 0;
}