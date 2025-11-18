#pragma once

#include <string>

namespace http {

struct Header {
    std::string name, value;
};

} // namespace http