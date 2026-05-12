#include "server/Server.h"
#include "mem/MemoryPool.h"
#include "util/Logger.h"

int main() {
    initMemoryPools();
    Logger::instance().setLevel(LogLevel::Info);

    constexpr int kPort = 8080;
    constexpr int kNumThreads = 4;

    Server server(kPort, kNumThreads);
    server.start();

    return 0;
}
