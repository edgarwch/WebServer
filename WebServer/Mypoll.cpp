#include "Mypoll.h"

Mypoll::Mypoll():_epollfd(epoll_create1(EPOLL_CLOEXEC)), _events(4096){
    if(_epollfd < 0){
        perror("epoll_create1 error");
        exit(-1);
    }
}


Mypoll::~Mypoll(){
}

void Mypoll::epoll_add(const spChannel& request){
    int fd = request->Getfd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->events();
    _fd2chan[fd] = request;
    request->UpdateLastEvents();
    if(epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &event) < 0){
        perror("epoll_add error");
        _fd2chan[fd].reset();
    }
}

void Mypoll::epoll_mod(const spChannel& request){
    int fd = request->Getfd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->events();
    if(!request->GetLastEvents() == request->events()){
        if (epoll_ctl(_epollfd, EPOLL_CTL_MOD, fd, &event)){
            perror("epoll_mod error");
            _fd2chan[fd].reset();
        }
    }
    request->UpdateLastEvents();
    
}

void Mypoll::epoll_del(const spChannel& request){
    int fd = request->Getfd();
    struct epoll_event event;
    event.data.fd = fd;
    event.events = request->events();
    if( epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, &event) < 0){
        perror("epoll_del error");
    }
    _fd2chan[fd].reset();

}

void Mypoll::poll(std::vector<spChannel>& request){
    int event_count = epoll_wait(_epollfd, &*_events.begin(), _events.size(), _epoll_wait_time_ms);
    for(int i = 0; i < event_count; ++i){
        int fd = _events[i].data.fd;
        spChannel tmp = _fd2chan[fd];
        tmp->SetRevents(_events[i].events);
        // avoid copy constructor
        request.emplace_back(tmp);
    }

}