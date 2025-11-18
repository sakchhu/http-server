#include "request.hpp"
#include "request_parser.hpp"

#include <cx/logger.hpp>

#include <iostream>
#include <print>

int main(int argc, char* argv[]) {
    // if (argc != 2) {
    //     std::println("usage: parse <request-string>");
    //     return -1;
    // }

    auto logger = cx::Logger{std::cout};
    auto req = http::Request{};
    auto parser = http::RequestParser{};

    const auto request_str =
        "GET / HTTP/1.0\r\nX-Custom-Header: 1234567890\r\n\r\n";
    auto res = parser.parse(req, request_str);

    switch (res) {
    case http::RequestParser::Result::Indeterminate:
        logger.warn("Incomplete Request String!");
        break;
    case http::RequestParser::Result::Invalid:
        logger.error("Invalid Request String!");
        break;
    default:
        break;
    }

    logger.info("{} -> \nMethod: {}\nURI: {}\nHTTP {}.{}", request_str,
                req.method, req.uri, req.version.major, req.version.minor);

    for (auto& header : req.headers) {
        std::println("{}: {}", header.name, header.value);
    }
}