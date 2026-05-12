#include "Channel.h"
#include "../util/Logger.h"

void Channel::handleEvents() {
    // Hang-up without data to read: connection closed
    if ((revents_ & EPOLLHUP) && !(revents_ & EPOLLIN)) {
        if (errorHandler_) {
            errorHandler_();
        }
        return;
    }

    if (revents_ & EPOLLERR) {
        if (errorHandler_) {
            errorHandler_();
        }
        return;
    }

    if (revents_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
        if (readHandler_) {
            readHandler_();
        }
    }

    if (revents_ & EPOLLOUT) {
        if (writeHandler_) {
            writeHandler_();
        }
    }
}
