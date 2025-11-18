#pragma once

#include "request_handler.hpp"

#include <asio/io_context.hpp>
#include <asio/ip/tcp.hpp>
#include <asio/signal_set.hpp>

namespace http {

class Server {
    using tcp = asio::ip::tcp;

  public:
    explicit Server(std::string address, std::string port, std::string doc_root,
                    std::size_t thread_pool_size);

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    void listen_and_serve();

  private:
    void do_accept();     // listen and accept connections
    void do_await_stop(); // wait for server stop signal

    asio::io_context context;

    // configure signals that will initiate server shutdown
    asio::signal_set signals;

    tcp::acceptor acceptor;

    std::size_t thread_pool_size;
    RequestHandler handler;
};

} // namespace http