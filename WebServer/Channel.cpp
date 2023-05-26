#include "Channel.h"


Channel::Channel(): _fd(-1), _events(0), _revents(0), _last_events(0){

}

Channel::Channel(int fd): _fd(fd), _events(0), _revents(0), _last_events(0){

}

Channel::~Channel(){

}

void Channel::HandleEvents(){
    _events = 0;
    // hang up event
    if((_revents & EPOLLHUP) && !(_revents & EPOLLIN)){
        _events = 0;
        return;
    }
    // error
    if(_revents & EPOLLERR){
        HandleError();
        _events = 0;
        return;
    }
    // read event or hight priority or read a hang up request
    if(_revents &(EPOLLIN | EPOLLPRI | EPOLLRDHUP)){
        HandleRead();
    }

    // write event
    if(_revents & EPOLLOUT){
        HandleWrite();
    }

    HandleUpdate();
}

void Channel::HandleWrite(){
    if(_write_handler){
        _write_handler();
    }
}

void Channel::HandleUpdate(){
    if(_update_handler){
        _update_handler();
    }
}

void Channel::HandleError(){
    if(_error_handler){
        _error_handler();
    }
}

void Channel::HandleRead(){
    if(_read_handler){
        _read_handler();
    }
}
