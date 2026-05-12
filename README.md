# WebServer

Multithreaded HTTP server using the Reactor pattern with epoll (Linux-only, C++20).

## Build

```bash
cmake -B build && cmake --build build
```

Requires `g++` (13+) and `cmake` (3.20+).

## Run

```bash
./build/webserver
```

Listens on port **8080** with 4 worker threads.

## Test

```bash
curl http://localhost:8080/
curl -X POST -d "hello" http://localhost:8080/
```

The server responds with an HTML page showing the request method, path, headers, and body.

## Configure

Edit `src/main.cpp` to change the port or thread count:

```cpp
constexpr int kPort = 8080;
constexpr int kNumThreads = 4;
```
