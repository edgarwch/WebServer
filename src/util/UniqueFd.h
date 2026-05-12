#pragma once

#include <unistd.h>

class UniqueFd {
public:
    constexpr UniqueFd() noexcept = default;
    explicit constexpr UniqueFd(int fd) noexcept : fd_(fd) {}

    ~UniqueFd() { reset(); }

    UniqueFd(const UniqueFd&) = delete;
    UniqueFd& operator=(const UniqueFd&) = delete;

    UniqueFd(UniqueFd&& other) noexcept : fd_(other.release()) {}
    UniqueFd& operator=(UniqueFd&& other) noexcept {
        reset(other.release());
        return *this;
    }

    void reset(int fd = -1) noexcept {
        if (fd_ >= 0) ::close(fd_);
        fd_ = fd;
    }

    [[nodiscard]] int release() noexcept {
        int fd = fd_;
        fd_ = -1;
        return fd;
    }

    [[nodiscard]] int get() const noexcept { return fd_; }
    explicit operator bool() const noexcept { return fd_ >= 0; }

private:
    int fd_ = -1;
};
