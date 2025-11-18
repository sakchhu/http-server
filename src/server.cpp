#include "server.hpp"
#include "connection.hpp"

#include <asio/strand.hpp>
#include <signal.h>
#include <thread>

namespace http {

Server::Server(std::string address, std::string port, std::string doc_root,
               std::size_t thread_pool_size)
    : signals{context}, acceptor{context}, thread_pool_size{thread_pool_size},
      handler{doc_root} {
    signals.add(SIGINT);
    signals.add(SIGTERM);

    do_await_stop();

    auto resolver = asio::ip::tcp::resolver{context};
    asio::ip::tcp::endpoint endpoint = *resolver.resolve(address, port).begin();

    acceptor.open(endpoint.protocol());
    acceptor.set_option(asio::ip::tcp::acceptor::reuse_address{true});
    acceptor.bind(endpoint);
    acceptor.listen();

    do_accept();
}

void Server::listen_and_serve() {
    std::vector<std::thread> threads;
    for (auto i = 0uz; i < this->thread_pool_size; ++i) {
        threads.emplace_back([this]() { context.run(); });
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

void Server::do_accept() {
    acceptor.async_accept(
        asio::make_strand(context),
        [this](asio::error_code err, asio::ip::tcp::socket socket) {
            //  if server was stopped by a signal, don't try to create more
            //  connections
            if (!acceptor.is_open()) {
                return;
            }

            if (!err) {
                std::make_shared<Connection>(std::move(socket), handler)
                    ->start();
            }
            do_accept();
        });
}

void Server::do_await_stop() {
    signals.async_wait(
        [this](asio::error_code /*err*/, int /*signal*/) { context.stop(); });
}

} // namespace http