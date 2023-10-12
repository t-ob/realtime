#include <iostream>

#include "Session.h"


Session::Session(tcp::socket socket, DisconnectCallback on_disconnect)
        : ws_(std::move(socket)), on_disconnect_(on_disconnect) {}

void Session::run() {
    // Perform the WebSocket handshake
    ws_.async_accept(
            beast::bind_front_handler(
                    &Session::on_accept,
                    shared_from_this()));
}

void Session::on_accept(beast::error_code ec) {
    if (ec) {
        handle_error(ec);
        return;
    }
}



void Session::send(const std::vector<uint8_t> &res) {
    ws_.binary(true);
    ws_.async_write(
        net::buffer(res),
        [sp = shared_from_this()](boost::system::error_code ec, std::size_t) {
            if (ec) {
                sp->handle_error(ec);
                return;
            }
        });
}

void Session::handle_error(beast::error_code ec) {
    std::cerr << "WebSocket error: " << ec.message() << "\n";
    on_disconnect_(shared_from_this());
}
