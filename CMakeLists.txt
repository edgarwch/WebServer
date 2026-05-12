cmake_minimum_required(VERSION 3.20)
project(WebServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_compile_options(-Wall -Wextra -Wpedantic -Wno-unused-parameter)

add_executable(webserver
    src/main.cpp
    src/server/Server.cpp
    src/core/EventLoop.cpp
    src/core/EventLoopThreadPool.cpp
    src/core/Channel.cpp
    src/core/Poller.cpp
    src/core/TimerQueue.cpp
    src/net/SocketOps.cpp
    src/net/HttpConnection.cpp
    src/net/HttpContext.cpp
    src/mem/MemoryPool.cpp
    src/util/Logger.cpp
)

target_link_libraries(webserver PRIVATE pthread)
target_include_directories(webserver PRIVATE src)
