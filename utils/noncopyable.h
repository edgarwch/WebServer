#pragma once

class noncopyable{
public:
    noncopyable();
    ~noncopyable();
private:
    // disable copy constructor and assign
    noncopyable(const noncopyable&);
    const noncopyable& operator=(const noncopyable&);
};