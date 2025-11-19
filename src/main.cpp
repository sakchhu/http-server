#include "server.hpp"
#include <cx/logger.hpp>

#include <iostream>

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::println(
            "usage: server <address> <port> <document-root> <threads>");
        return 1;
    }

    const auto address = argv[1];
    const auto port = argv[2];
    const auto doc_root = argv[3];
    auto thread_pool_size = std::atoi(argv[4]);
    if (thread_pool_size <= 0) {
        thread_pool_size = 1; // at least 1 thread so the context can run :3
    }

    auto logger = cx::Logger{std::cout};

    using namespace http;

    try {
        Server server{address, port, doc_root,
                      static_cast<size_t>(thread_pool_size)};
        server.listen_and_serve();
    } catch (const std::exception& e) {
        logger.error("{}", e.what());
    }
}