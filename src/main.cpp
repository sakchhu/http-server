#include "server.hpp"
#include <cx/logger.hpp>

#include <iostream>

int main(int argc, char* argv[]) {
    auto logger = cx::Logger{std::cout};

    using namespace http;

    try {
        Server server{"127.0.0.1", "8000", "public", 2};
        server.listen_and_serve();
    } catch (const std::exception& e) {
        logger.error("{}", e.what());
    }
}