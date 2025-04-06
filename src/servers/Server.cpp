#include "Server.h"
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <csignal>
#include "../parser/RedisParser.h"

void Server::Start() {
    signal(SIGPIPE, SIG_IGN);
    std::cout << "Server running on port " << socket_.GetPort() << "..." << std::endl;
    event_loop_.Run();
}

Server::Server(const int port): socket_(port), event_loop_(1000) {
    auto serverEventHandler = [this](const int fd, const uint32_t events) {
        HandleServerEvent(fd, events);
    };
    event_loop_.AddFd(socket_.GetServerFd(), EPOLLIN | EPOLLET, serverEventHandler);
}

void Server::HandleServerEvent(const int server_fd, const uint32_t events) {
    if (events & EPOLLIN) {
        const int client_fd = Socket::AcceptConnection(server_fd);
        auto clientEventHandler = [this](const int fd, const uint32_t events) {
            HandleClientEvent(fd, events);
        };
        event_loop_.AddFd(client_fd, EPOLLIN | EPOLLET, clientEventHandler);
    }
}

void Server::HandleClientEvent(const int client_fd, const uint32_t events) {
    if (events & EPOLLIN) {
        char buffer[1024]{};
        while (read(client_fd, buffer, sizeof(buffer)) > 0) {
            std::string response = redis_parser_.HandleCommand(buffer);
            write(client_fd, response.c_str(), response.size());
        }
    }
}
