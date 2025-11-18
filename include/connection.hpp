#pragma once

#include "request.hpp"
#include "request_handler.hpp"
#include "request_parser.hpp"
#include "response.hpp"

#include <asio/ip/tcp.hpp>

namespace http {

class Connection : public std::enable_shared_from_this<Connection> {
  public:
    explicit Connection(asio::ip::tcp::socket socket, RequestHandler& handler);

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    void start();

  private:
    void do_write();
    void do_read();

    asio::ip::tcp::socket socket;
    constexpr static inline std::size_t buffer_max_size = 1024;
    std::string buffer = std::string(buffer_max_size, '\0');

    Request request;
    RequestParser parser;
    RequestHandler& handler;

    Response response;
};

using ConnectionPtr = std::shared_ptr<Connection>;

} // namespace http