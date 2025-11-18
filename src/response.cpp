#include "response.hpp"

#include <asio.hpp>
#include <format>
#include <string>
#include <utility>

namespace http {

std::string_view to_string(Response::StatusCode code) {
    switch (code) {
        using enum Response::StatusCode;
    case Ok:
        return "Ok";
    case Created:
        return "Created";
    case Accepted:
        return "Accepted";
    case NoContent:
        return "No Content";
    case MultipleChoices:
        return "Multiple Choices";
    case MovedPermanently:
        return "Moved Permanently";
    case MovedTemporarily:
        return "Moved Temporarily";
    case NotModified:
        return "Not Modified";
    case BadRequest:
        return "Bad Request";
    case Unauthorized:
        return "Unauthorized";
    case Forbidden:
        return "Forbidden";
    case NotFound:
        return "Not Found";
    case InternalServerError:
        return "Internal Server Error";
    case NotImplemented:
        return "Not Implemented";
    case BadGateway:
        return "Bad Gateway";
    case ServiceUnavailable:
        return "Service Unavailable";
    default:
        return "Internal Server Error";
        break;
    }
}

namespace status_line {
std::string create(Response::StatusCode code) {
    return std::format("HTTP/1.0 {} {}\r\n", std::to_underlying(code),
                       to_string(code));
}
} // namespace status_line

namespace content {
std::string create(Response::StatusCode code) {
    static constexpr auto fmt = "<html>"
                                "<head><title>{1}</title></head>"
                                "<body><h1>{0} {1}</h1></body>"
                                "</html>";
    return std::format(fmt, std::to_underlying(code), to_string(code));
}
} // namespace content

auto Response::to_buffers() -> std::vector<asio::const_buffer> {
    static constexpr std::string_view header_separator_v = ": ";
    static constexpr std::string_view crlf_v = "\r\n";

    status_line = status_line::create(status);

    std::vector<asio::const_buffer> buffers;
    buffers.push_back(asio::buffer(status_line));
    for (const auto& header : headers) {
        buffers.push_back(asio::buffer(header.name));
        buffers.push_back(asio::buffer(header_separator_v));
        buffers.push_back(asio::buffer(header.value));
        buffers.push_back(asio::buffer(crlf_v));
    }
    buffers.push_back(asio::buffer(crlf_v));
    buffers.push_back(asio::buffer(content));
    return buffers;
}

Response Response::from(Response::StatusCode code) {
    Response ret{
        .status = code,
        .content = content::create(code),
    };
    ret.headers.push_back(Header{
        "Content-Length",
        std::to_string(ret.content.size()),
    });
    ret.headers.push_back(Header{"Content-Type", "text/html"});
    return ret;
}

} // namespace http