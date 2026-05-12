# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build & Run

```bash
# Using CMake (recommended):
cmake -B build && cmake --build build
./build/webserver

# Manual compile with g++ (C++20 required):
g++ -std=c++20 -Wall -Wextra -Wpedantic -Wno-unused-parameter \
  src/main.cpp \
  src/server/Server.cpp \
  src/core/EventLoop.cpp \
  src/core/EventLoopThreadPool.cpp \
  src/core/Channel.cpp \
  src/core/Poller.cpp \
  src/core/TimerQueue.cpp \
  src/net/SocketOps.cpp \
  src/net/HttpConnection.cpp \
  src/net/HttpContext.cpp \
  src/mem/MemoryPool.cpp \
  src/util/Logger.cpp \
  -I src -o server -lpthread
```

The server listens on port 8080 with 4 worker threads by default.

## Architecture

This is a **multithreaded HTTP server** using the **Reactor pattern** with epoll (Linux-only). One EventLoop per thread, with round-robin distribution of connections.

**Startup flow**: `main` → `Server(port, numThreads)` binds the listen socket and creates an `EventLoopThreadPool` with N workers → `server.start()` launches all event loop threads (each runs `EventLoop::loop()`), then the main thread loops on `accept4()`. Each accepted connection is wrapped in an `HttpConnection` and dispatched to the next EventLoop via `RunInLoop()` (round-robin).

**Event loop thread flow**: `epoll_wait` → collect ready `Channel` objects → dispatch `handleEvents()` on each → process expired timers → process the pending function queue. Loop exits when `std::stop_token` signals stop.

**Key classes**:
- `EventLoop` — per-thread reactor; owns a `Poller` (epoll wrapper), an eventfd (via `UniqueFd`) for async wakeup, a `TimerQueue`, and a pending-function queue. Uses `std::stop_token` for graceful shutdown.
- `Channel` — wraps an fd with read/write/error callbacks; uses `std::enable_shared_from_this`
- `EventLoopThreadPool` — owns N `std::jthread` + `EventLoop` workers; uses lock-free `std::atomic<size_t>` round-robin
- `Poller` — epoll wrapper mapping fd → Channel via `std::unordered_map`; fixes `!=` bug from old Mypoll
- `MemoryPool` — fixed-size slab allocator with 64 size classes (8 to 512 bytes); allocations >512B fall through to `operator new`. Use `NewElement<T>(args...)` / `DeleteElement<T>(ptr)` templates.
- `TimerQueue` — min-heap of timers with `std::mutex`
- `HttpConnection` — per-connection state machine owning Channel + UniqueFd + HttpContext; uses `weak_from_this()` for safe callback lifetime
- `HttpContext` — HTTP/1.1 request parser (request line, headers, body)
- `UniqueFd` — RAII move-only file descriptor wrapper; closes fd in destructor
- `Logger` — thread-safe structured logging with timestamps and log levels

## Project layout

```
src/
  main.cpp                      — entry point
  util/
    Logger.h / .cpp             — structured logging
    UniqueFd.h                  — RAII file descriptor wrapper
  core/
    Poller.h / .cpp             — epoll wrapper
    Channel.h / .cpp            — fd + callback dispatcher
    EventLoop.h / .cpp          — per-thread reactor
    EventLoopThreadPool.h / .cpp — jthread pool with lock-free round-robin
    TimerQueue.h / .cpp         — min-heap timer queue
  net/
    SocketOps.h / .cpp          — socket utilities (readn, writen, createAndListen)
    HttpContext.h / .cpp        — HTTP request parser
    HttpConnection.h / .cpp     — per-connection state machine
  mem/
    MemoryPool.h / .cpp         — slab allocator
  server/
    Server.h / .cpp             — accept loop + dispatch
CMakeLists.txt                  — CMake build (C++20)
```

## Coding style

- C++20 with `-Wall -Wextra -Wpedantic`
- `using` aliases, not `typedef`
- Default member initializers, not constructor init lists for trivial values
- `= delete` copy instead of `noncopyable` base class
- `std` primitives for sync (`std::mutex`, `std::lock_guard`, `std::jthread`, `std::stop_token`)
- RAII for all resources (file descriptors, memory, epoll handles)
- Lambdas, never `std::bind`
- `static_cast` / `reinterpret_cast`, never C-style casts
- `constexpr` for compile-time constants
- `[[nodiscard]]` on functions where ignoring the return value is a bug
- `noexcept` on move constructors and trivial accessors
