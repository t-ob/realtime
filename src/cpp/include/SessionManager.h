#pragma once

#include <set>

#include "Session.h"

namespace beast = boost::beast;
using tcp = boost::asio::ip::tcp;

class Session;


class SessionManager : public std::enable_shared_from_this<SessionManager> {
    std::set<std::shared_ptr<Session>> sessions_;
    std::mutex mutex_;
    tcp::acceptor acceptor_;

public:
    SessionManager(boost::asio::io_context& ioc);
    void broadcast(std::vector<uint8_t> &buf);

    void start();

    void do_accept();

    void on_accept(beast::error_code ec, tcp::socket socket);

    void remove_session(std::shared_ptr<Session> session);
};