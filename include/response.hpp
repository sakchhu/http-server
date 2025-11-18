#pragma once

#include "header.hpp"

#include <asio/buffer.hpp>
#include <string>
#include <vector>

namespace http {

class Response {
  public:
    enum class StatusCode {
        // Successful 2xx
        Ok = 200,
        Created = 201,
        Accepted = 202,
        NoContent = 204,
        // Redirection 3xx
        MultipleChoices = 300,
        MovedPermanently = 301,
        MovedTemporarily = 302,
        NotModified = 304,
        // Client Error 4xx
        BadRequest = 400,
        Unauthorized = 401,
        Forbidden = 403,
        NotFound = 404,
        // Server Error 5xx
        InternalServerError = 500,
        NotImplemented = 501,
        BadGateway = 502,
        ServiceUnavailable = 503,
    };

    // create a generic Response from a status code
    static Response from(StatusCode code);
    auto to_buffers() -> std::vector<asio::const_buffer>;

    StatusCode status;
    // note: asio::buffers are non-owning views, so response information has to
    // be able to outlive the asio::buffers
    std::string status_line;
    std::string content;
    std::vector<Header> headers;
};

std::string_view to_string(Response::StatusCode code);

} // namespace http