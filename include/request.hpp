#pragma once

#include "header.hpp"
#include <vector>

namespace http {

struct Request {
    std::string method, uri;
    struct HttpVersion {
        int major = {};
        int minor = {};
    } version;
    std::vector<http::Header> headers;
};

} // namespace http