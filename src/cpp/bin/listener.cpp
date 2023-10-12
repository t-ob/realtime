#include <vector>
#include <iostream>
#include <memory>
#include <sys/un.h>
#include <sys/socket.h>
#include <thread>


#include "SessionManager.h"

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>

void insert_ringbuf(std::vector<uint8_t> &ring_buf, std::vector<size_t> &ptrs, const uint8_t *buf, size_t len) {
    auto recv_ptr = ptrs[0];
    auto write_ptr = ptrs[1];

    auto capacity = recv_ptr < write_ptr ? write_ptr - recv_ptr : ring_buf.size() - (recv_ptr - write_ptr);

    if (capacity < len) {
        std::cerr << "Cannot write beyond write ptr" << std::endl;
        return;
    }

    auto mask = ring_buf.size() - 1;

    for (auto i = 0; i < len; ++i) ring_buf[(recv_ptr + i) & mask] = buf[i];

    ptrs[0] = (recv_ptr + len) & mask;
}

void pop_ringbuf(std::vector<uint8_t> &ring_buf, std::vector<size_t> &ptrs, std::vector<uint8_t> &res) {
    auto recv_ptr = ptrs[0];
    auto write_ptr = ptrs[1];

    auto mask = ring_buf.size() - 1;

    if (write_ptr == recv_ptr) {
        std::cerr << "Nothing to pop" << std::endl;
        return;
    }

    uint32_t payload_len;
    std::memcpy(&payload_len, ring_buf.data() + write_ptr, sizeof(uint32_t));

    for (auto i = sizeof(uint32_t); i < payload_len; ++i) res.push_back(ring_buf[(write_ptr + i) & mask]);

    ptrs[1] = (write_ptr + payload_len) & mask;
}


void tensor_listener(std::shared_ptr<std::vector<uint8_t>> ring_buf, std::shared_ptr<std::vector<size_t>> ptrs,
                     const char *socket_path) {
    // Create a socket
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_fd == -1) {
        std::cerr << "Error creating socket" << std::endl;
        return;
    }

    sockaddr_un address{};
    std::memset(&address, 0, sizeof(address));
    address.sun_family = AF_UNIX;
    std::strncpy(address.sun_path, socket_path, sizeof(address.sun_path) - 1);

    // Bind the socket to the path
    unlink(socket_path); // Remove any existing socket at this path
    if (bind(server_fd, (struct sockaddr *) &address, sizeof(address)) == -1) {
        std::cerr << "Error binding socket" << std::endl;
        close(server_fd);
        return;
    }

    // Start listening for connections
    if (listen(server_fd, 5) == -1) {
        std::cerr << "Error listening on socket" << std::endl;
        close(server_fd);
        return;
    }

    std::cout << "Listening for connections on " << socket_path << std::endl;

    while (true) {
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd == -1) {
            std::cerr << "Error accepting connection" << std::endl;
            break;
        }

        uint8_t buffer[1 << 12];
        ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer), 0);
        if (bytes_received <= 0) {
            std::cerr << "Error receiving data or client disconnected" << std::endl;
        } else if (bytes_received >= sizeof(buffer)) {
            std::cerr << "Payload too large" << std::endl;
        } else {
            std::cout << "bytes received: " << bytes_received << std::endl << "Received: " << buffer << std::endl;
            insert_ringbuf(*ring_buf, *ptrs, buffer, bytes_received);
        }

        std::cout << (*ptrs)[0] << std::endl;

        close(client_fd);
    }

    close(server_fd);
    unlink(socket_path); // Clean up the socket file
}

void broadcast_async(const boost::system::error_code & /*e*/,
                     boost::asio::steady_timer *t, std::shared_ptr<SessionManager> manager,
                     std::shared_ptr<std::vector<uint8_t>> ring_buf, std::shared_ptr<std::vector<size_t>> ptrs) {
    while ((*ptrs)[0] != (*ptrs)[1]) {
        std::cout << "broadcasting" << std::endl;
        std::vector<uint8_t> res;
        pop_ringbuf(*ring_buf, *ptrs, res);
        manager->broadcast(res);
    }

    t->expires_at(t->expiry() + boost::asio::chrono::milliseconds (1));
    t->async_wait(boost::bind(broadcast_async,
                              boost::asio::placeholders::error, t, manager, ring_buf, ptrs
    ));
}

void websocket_listener(std::shared_ptr<std::vector<uint8_t>> ring_buf, std::shared_ptr<std::vector<size_t>> ptrs) {
    boost::asio::io_context ioc;
    auto manager = std::make_shared<SessionManager>(ioc);

    manager->start();

    boost::asio::steady_timer timer(ioc, boost::asio::chrono::milliseconds(1));
    timer.async_wait(boost::bind(broadcast_async,
                                 boost::asio::placeholders::error, &timer, manager, ring_buf, ptrs
    ));

    ioc.run();
}

int main() {
    const char *SOCKET_PATH = "/tmp/example_socket";
    const size_t BUF_SIZE = 1 << 20;

    auto ptrs = std::make_shared<std::vector<size_t>>(2, 0);
    auto ring_buf = std::make_shared<std::vector<uint8_t>>(BUF_SIZE, 0);

    std::thread t1(tensor_listener, ring_buf, ptrs, SOCKET_PATH);
    std::thread t2(websocket_listener, ring_buf, ptrs);

    t1.join();
    t2.join();

    return 0;
}