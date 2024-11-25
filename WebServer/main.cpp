#include "Server.h"

int main() {
    const int port = 8080;        // Port to listen on
    const int numThreads = 2;    // Number of threads in the thread pool

    Server server(port, numThreads);
    server.start(); // Start the server

    return 0;
}
