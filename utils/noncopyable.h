#pragma once

class noncopyable{
public:
    noncopyable() = default;
    ~noncopyable() = default;
private:
    // disable copy constructor and assign
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;
};