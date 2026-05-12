#pragma once

#include "../util/UniqueFd.h"
#include <string>
#include <cstddef>
#include <sys/types.h>

// Read/write with partial I/O, EINTR, and EAGAIN handling
ssize_t readn(int fd, void* buf, size_t n);
ssize_t readn(int fd, std::string& inBuffer, bool& zero);
ssize_t readn(int fd, std::string& inBuffer);
ssize_t writen(int fd, const void* buf, size_t n);
ssize_t writen(int fd, std::string& sbuf);

void setNonBlocking(int fd);
void setNoDelay(int fd);
void setNoLinger(int fd);
void shutdownWrite(int fd);

[[nodiscard]] UniqueFd createAndListen(int port);
