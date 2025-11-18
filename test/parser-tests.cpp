#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include "request.hpp"
#include "request_parser.hpp"

#include <string_view>

using http::Request;
using http::RequestParser;

static auto header_value(const Request& req, std::string_view name)
    -> std::string {
    for (auto& h : req.headers) {
        if (h.name == name)
            return h.value;
    }
    return {};
}

TEST_CASE("HTTP/1.0 Parser - complete request") {
    RequestParser parser;
    Request req;

    constexpr std::string_view input = "GET /index.html HTTP/1.0\r\n"
                                       "Host: example.com\r\n"
                                       "User-Agent: Test\r\n"
                                       "\r\n";

    auto result = parser.parse(req, input);
    REQUIRE(result == RequestParser::Result::Complete);

    REQUIRE(req.method == "GET");
    REQUIRE(req.uri == "/index.html");
    REQUIRE(req.version.major == 1);
    REQUIRE(req.version.minor == 0);

    REQUIRE(header_value(req, "Host") == "example.com");
    REQUIRE(header_value(req, "User-Agent") == "Test");
    REQUIRE(req.headers.size() == 2);
}

TEST_CASE("HTTP/1.0 Parser - incomplete (Indeterminate) request") {
    RequestParser parser;
    Request req;

    SECTION("Partial request line") {
        constexpr std::string_view input = "GET /index.html HT";
        auto result = parser.parse(req, input);
        REQUIRE(result == RequestParser::Result::Indeterminate);
    }

    SECTION("Headers partially received") {
        constexpr std::string_view input = "GET /index.html HTTP/1.0\r\n"
                                           "Host: example";
        auto result = parser.parse(req, input);
        REQUIRE(result == RequestParser::Result::Indeterminate);
    }

    SECTION("Missing final CRLF") {
        constexpr std::string_view input =
            "GET /index.html HTTP/1.0\r\n"
            "Host: example.com\r\n"
            "User-Agent: Test\r\n"; // no terminating CRLF
        auto result = parser.parse(req, input);
        REQUIRE(result == RequestParser::Result::Indeterminate);
    }
}

TEST_CASE("HTTP/1.0 Parser - invalid requests") {
    RequestParser parser;
    Request req;

    SECTION("Invalid request line") {
        constexpr std::string_view input = "GARBAGE\r\n\r\n";
        auto result = parser.parse(req, input);
        REQUIRE(result == RequestParser::Result::Invalid);
    }

    SECTION("Missing HTTP version") {
        constexpr std::string_view input = "GET /index.html\r\n\r\n";
        auto result = parser.parse(req, input);
        REQUIRE(result == RequestParser::Result::Invalid);
    }

    SECTION("Malformed header") {
        constexpr std::string_view input = "GET /index.html HTTP/1.0\r\n"
                                           "BadHeader\r\n"
                                           "\r\n";
        auto result = parser.parse(req, input);
        REQUIRE(result == RequestParser::Result::Invalid);
    }
}

TEST_CASE("HTTP/1.0 Parser - multiple incremental chunks") {
    RequestParser parser;
    Request req;

    std::vector<std::string> chunks = {
        "GET /index.html HT", "TP/1.0\r\nHost: exa",
        "mple.com\r\nUser-Agent: T", "est\r\n\r\n"};

    RequestParser::Result r = RequestParser::Result::Indeterminate;

    for (size_t i = 0; i < chunks.size(); i++) {
        r = parser.parse(req, chunks[i]);

        if (i < chunks.size() - 1) {
            REQUIRE(r == RequestParser::Result::Indeterminate);
        }
    }

    REQUIRE(r == RequestParser::Result::Complete);

    REQUIRE(req.method == "GET");
    REQUIRE(req.uri == "/index.html");
    REQUIRE(req.version.major == 1);
    REQUIRE(req.version.minor == 0);

    REQUIRE(header_value(req, "Host") == "example.com");
    REQUIRE(header_value(req, "User-Agent") == "Test");
}

TEST_CASE("HTTP/1.0 Parser - empty input") {
    RequestParser parser;
    Request req;

    auto r = parser.parse(req, "");
    REQUIRE(r == RequestParser::Result::Indeterminate);
}
