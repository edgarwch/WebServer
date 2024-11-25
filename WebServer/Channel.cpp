#include "Channel.h"
#include <iostream>

Channel::Channel() 
    : _fd(-1), _events(0), _revents(0), _last_events(0) {}

Channel::Channel(int fd)
    : _fd(fd), _events(0), _revents(0), _last_events(0) {}

Channel::~Channel() {}

void Channel::HandleEvents() {
    if ((_revents & EPOLLHUP) && !(_revents & EPOLLIN)) {
        if (_error_handler) {
            _error_handler();
        } else {
            std::cerr << "[ERROR] No error handler set for Channel with fd: " << _fd << std::endl;
        }
        return;
    }
    if (_revents & EPOLLERR) {
        if (_error_handler) {
            _error_handler();
        } else {
            std::cerr << "[ERROR] No error handler set for Channel with fd: " << _fd << std::endl;
        }
        return;
    }
    if (_revents & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (_read_handler) {
            _read_handler();
        } else {
            std::cerr << "[ERROR] No read handler set for Channel with fd: " << _fd << std::endl;
        }
    }
    if (_revents & EPOLLOUT) {
        if (_write_handler) {
            _write_handler();
        } else {
            std::cerr << "[ERROR] No write handler set for Channel with fd: " << _fd << std::endl;
        }
    }
}


void Channel::HandleWrite() {
    if (_write_handler) {
        _write_handler();
    }
}

void Channel::HandleUpdate() {
    if (_update_handler) {
        _update_handler();
    }
}

void Channel::HandleError() {
    if (_error_handler) {
        _error_handler();
    }
}

void Channel::HandleRead() {
    if (_read_handler) {
        _read_handler();
    }
}
