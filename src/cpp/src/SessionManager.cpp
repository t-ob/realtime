#include <memory>
#include <thread>
#include <vector>
#include "SessionManager.h"

#include <iostream>

// TODO: verbose logging only when specified

void SessionManager::start() {
    auto const address = net::ip::make_address("0.0.0.0");  // TODO: parametrise
    auto const port = static_cast<unsigned short>(9001);  // TODO: parametrise

    tcp::endpoint endpoint{address, port};

    beast::error_code ec;
    acceptor_.open(endpoint.protocol(), ec);
    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    acceptor_.bind(endpoint, ec);
    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        throw beast::system_error(ec);
    }

    do_accept();
}

void SessionManager::do_accept() {
    acceptor_.async_accept(
            beast::bind_front_handler(&SessionManager::on_accept, shared_from_this()));
}

void SessionManager::on_accept(beast::error_code ec, tcp::socket socket) {
    if (!ec) {
        std::lock_guard<std::mutex> lock(mutex_);
        auto session = std::make_shared<Session>(std::move(socket), [this](std::shared_ptr<Session> session_to_remove) {
            this->remove_session(session_to_remove);
        });
        sessions_.insert(session);
        session->run();
    }
    do_accept();
}

void SessionManager::broadcast(std::vector<uint8_t> &buf) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::cout << "Broadcasting to " << sessions_.size() << " sessions." << std::endl;
    for (auto &session: sessions_) {
        std::cout << "Sending to session..." << std::endl;
        session->send(buf);
    }
    std::cout << "Broadcast complete." << std::endl;
}

void SessionManager::remove_session(std::shared_ptr<Session> session) {
    std::lock_guard<std::mutex> lock(mutex_);
    sessions_.erase(session);
    std::cout << "Session removed. Total sessions: " << sessions_.size() << std::endl;
}

SessionManager::SessionManager(boost::asio::io_context &ioc) : acceptor_(ioc) {}
