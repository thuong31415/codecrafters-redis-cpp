#include "Socket.h"

#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

Socket::Socket(const uint16_t port): port_(port) {
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        throw std::runtime_error("Failed to create server socket");
    }

    constexpr int reuse = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        throw std::runtime_error("set sock_opt failed");
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd_, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) != 0) {
        throw std::runtime_error("Failed to bind to port " + std::to_string(port));
    }

    if (listen(server_fd_, SOMAXCONN) != 0) {
        throw std::runtime_error("listen failed");
    }
}

Socket::~Socket() {
    if (server_fd_ >= 0) {
        close(server_fd_);
    }
}

int Socket::AcceptConnection(const int server_fd) {
    sockaddr_in client_address{};
    socklen_t client_addr_len = sizeof(client_address);
    const int client_fd = accept(server_fd, reinterpret_cast<sockaddr *>(&client_address), &client_addr_len);
    if (client_fd < 0) {
        throw std::runtime_error("accept socket error");
    }
    return client_fd;
}

int Socket::GetServerFd() const {
    return server_fd_;
}

uint16_t Socket::GetPort() const {
    return port_;
}
