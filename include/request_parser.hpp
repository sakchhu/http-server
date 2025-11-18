#pragma once

#include <string_view>

namespace http {

struct Request;

class RequestParser {
  public:
    enum class Result {
        Indeterminate = -1,
        Complete,
        Invalid,
    };

    // Parse input and populate request data.
    // Returns Result::Indeterminate if the request data was not fully
    // populated by available input
    auto parse(Request& req, std::string_view input) noexcept -> Result;

    void reset() { state = {}; };

  private:
    auto consume(Request& req, char c) -> Result;

    enum class State {
        // GET /index.html
        MethodStart,
        Method,
        Uri,

        // HTTP/2.0
        HttpVersionH,
        HttpVersionT1,
        HttpVersionT2,
        HttpVersionP,
        HttpVersionSlash,
        HttpVersionMajorStart,
        HttpVersionMajor,
        HttpVersionMinorStart,
        HttpVersionMinor,
        HttpVersionEnd,

        HeaderLineStart,
        HeaderName,
        HeaderNameEnd,
        HeaderValue,
        HeaderValueEnd,

        RequestEnd,
    } state = {};
};

} // namespace http