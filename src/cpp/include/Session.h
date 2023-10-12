#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

#include "SessionManager.h"

namespace beast = boost::beast;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;


class Session : public std::enable_shared_from_this<Session> {

public:
    using DisconnectCallback = std::function<void(std::shared_ptr<Session>)>;

    explicit Session(tcp::socket socket, DisconnectCallback on_disconnect);

    void send(const std::vector<uint8_t> &res);

    void run();

    void on_accept(beast::error_code ec);

private:
    beast::websocket::stream<tcp::socket> ws_;
    DisconnectCallback on_disconnect_;

    void handle_error(beast::error_code ec);
};