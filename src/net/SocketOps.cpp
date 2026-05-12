#include "SocketOps.h"
#include "../util/Logger.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <unistd.h>

namespace {

constexpr size_t kMaxBuffer = 4096;

} // namespace

ssize_t readn(int fd, void* buf, size_t n) {
    size_t nleft = n;
    ssize_t nread = 0;
    ssize_t readSum = 0;
    auto* ptr = static_cast<char*>(buf);
    while (nleft > 0) {
        nread = ::read(fd, ptr, nleft);
        if (nread < 0) {
            if (errno == EINTR) {
                nread = 0;
            } else if (errno == EAGAIN) {
                return readSum;
            } else {
                return -1;
            }
        } else if (nread == 0) {
            break;
        }
        readSum += nread;
        nleft -= static_cast<size_t>(nread);
        ptr += nread;
    }
    return readSum;
}

ssize_t readn(int fd, std::string& inBuffer, bool& zero) {
    ssize_t nread = 0;
    ssize_t readSum = 0;
    while (true) {
        char buff[kMaxBuffer];
        nread = ::read(fd, buff, kMaxBuffer);
        if (nread < 0) {
            if (errno == EINTR) {
                continue;
            } else if (errno == EAGAIN) {
                return readSum;
            } else {
                Logger::error() << "readn failed: " << errno;
                return -1;
            }
        } else if (nread == 0) {
            zero = true;
            break;
        }
        readSum += nread;
        inBuffer.append(buff, static_cast<size_t>(nread));
    }
    return readSum;
}

ssize_t readn(int fd, std::string& inBuffer) {
    ssize_t nread = 0;
    ssize_t readSum = 0;
    while (true) {
        char buff[kMaxBuffer];
        nread = ::read(fd, buff, kMaxBuffer);
        if (nread < 0) {
            if (errno == EINTR) {
                continue;
            } else if (errno == EAGAIN) {
                return readSum;
            } else {
                Logger::error() << "readn failed: " << errno;
                return -1;
            }
        } else if (nread == 0) {
            break;
        }
        readSum += nread;
        inBuffer.append(buff, static_cast<size_t>(nread));
    }
    return readSum;
}

ssize_t writen(int fd, const void* buf, size_t n) {
    size_t nleft = n;
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    auto* ptr = static_cast<const char*>(buf);
    while (nleft > 0) {
        nwritten = ::write(fd, ptr, nleft);
        if (nwritten <= 0) {
            if (nwritten < 0) {
                if (errno == EINTR) {
                    nwritten = 0;
                    continue;
                } else if (errno == EAGAIN) {
                    return writeSum;
                } else {
                    return -1;
                }
            }
        }
        writeSum += nwritten;
        nleft -= static_cast<size_t>(nwritten);
        ptr += nwritten;
    }
    return writeSum;
}

ssize_t writen(int fd, std::string& sbuf) {
    size_t nleft = sbuf.size();
    ssize_t nwritten = 0;
    ssize_t writeSum = 0;
    const char* ptr = sbuf.data();
    while (nleft > 0) {
        nwritten = ::write(fd, ptr, nleft);
        if (nwritten <= 0) {
            if (nwritten < 0) {
                if (errno == EINTR) {
                    nwritten = 0;
                    continue;
                } else if (errno == EAGAIN) {
                    break;
                } else {
                    return -1;
                }
            }
        }
        writeSum += nwritten;
        nleft -= static_cast<size_t>(nwritten);
        ptr += nwritten;
    }
    if (writeSum == static_cast<ssize_t>(sbuf.size())) {
        sbuf.clear();
    } else {
        sbuf.erase(0, static_cast<size_t>(writeSum));
    }
    return writeSum;
}

void setNonBlocking(int fd) {
    int flag = ::fcntl(fd, F_GETFL, 0);
    if (flag == -1) return;
    ::fcntl(fd, F_SETFL, flag | O_NONBLOCK);
}

void setNoDelay(int fd) {
    int enable = 1;
    ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &enable, sizeof(enable));
}

void setNoLinger(int fd) {
    linger lin{};
    lin.l_onoff = 1;
    lin.l_linger = 30;
    ::setsockopt(fd, SOL_SOCKET, SO_LINGER, &lin, sizeof(lin));
}

void shutdownWrite(int fd) {
    ::shutdown(fd, SHUT_WR);
}

UniqueFd createAndListen(int port) {
    if (port < 0 || port > 65535) return UniqueFd{};

    int listenFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd < 0) return UniqueFd{};

    int optval = 1;
    if (::setsockopt(listenFd, SOL_SOCKET, SO_REUSEADDR, &optval,
                     sizeof(optval)) < 0) {
        ::close(listenFd);
        return UniqueFd{};
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(static_cast<uint16_t>(port));

    if (::bind(listenFd, reinterpret_cast<sockaddr*>(&serverAddr),
               sizeof(serverAddr)) < 0) {
        ::close(listenFd);
        return UniqueFd{};
    }

    if (::listen(listenFd, 2048) < 0) {
        ::close(listenFd);
        return UniqueFd{};
    }

    return UniqueFd(listenFd);
}
