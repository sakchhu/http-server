#include "connection.hpp"
#include <asio/write.hpp>

#include <cx/logger.hpp>
#include <iostream>

static auto connection_logger_v = cx::Logger{std::cout};

namespace http {

Connection::Connection(asio::ip::tcp::socket socket, RequestHandler& handler)
    : socket{std::move(socket)}, handler{handler} {}

void Connection::start() { do_read(); }

void Connection::do_read() {
    auto self{shared_from_this()};
    socket.async_read_some(
        asio::buffer(buffer.data(), buffer_max_size),
        [this, self](const asio::error_code& err, std::size_t bytes_read) {
            if (!err) {
                auto result =
                    parser.parse(request, {buffer.data(), bytes_read});
                switch (result) {
                case RequestParser::Result::Complete:
                    response = handler.handle(request);
                    parser.reset();
                    do_write();
                    break;
                case RequestParser::Result::Invalid: // respond with a generic
                                                     // Bad Request response
                    response = Response::from(Response::StatusCode::BadRequest);
                    parser.reset();
                    do_write();
                    break;
                case RequestParser::Result::Indeterminate: // keep reading
                    do_read();
                    break;
                }
                connection_logger_v.info(
                    "Received:\n{}",
                    std::string_view{buffer.data(), bytes_read});
            }
        });
}

void Connection::do_write() {
    auto self{shared_from_this()};
    asio::async_write(
        socket, response.to_buffers(),
        [this, self](asio::error_code err, std::size_t bytes_written) {
            if (!err) {
                std::ignore =
                    socket.shutdown(asio::ip::tcp::socket::shutdown_both, err);
                connection_logger_v.info("Connection Closed");
            }
        });
}

} // namespace http