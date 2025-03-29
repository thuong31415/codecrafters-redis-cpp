#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

int CreateSocket() {
    const int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        throw std::runtime_error("Failed to create server socket");
    }

    constexpr int reuse = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        throw std::runtime_error("set sock_opt failed");
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(6379);

    if (bind(server_fd, reinterpret_cast<sockaddr *>(&server_addr), sizeof(server_addr)) != 0) {
        throw std::runtime_error("Failed to bind to port 6379");
    }

    if (listen(server_fd, 5) != 0) {
        throw std::runtime_error("listen failed");
    }

    std::cout << "Waiting for a client to connect...\n";

    return server_fd;
}


int main() {
    const int server_fd = CreateSocket();

    sockaddr_in client_addr{};
    socklen_t client_addr_len = sizeof(client_addr);

    const int client_fd = accept(server_fd, reinterpret_cast<sockaddr *>(&client_addr), &client_addr_len);

    std::cout << "Client connected\n";

    const std::string response = "+PONG\r\n";

    write(client_fd, response.c_str(), response.length());

    close(server_fd);

    return 0;
}
