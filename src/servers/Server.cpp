#include "Server.h"
#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <csignal>
#include <RespParser.h>

void Server::Start() {
    signal(SIGPIPE, SIG_IGN);
    std::cout << "Server running on port " << socket_.GetPort() << "..." << std::endl;
    event_loop_.Run();
}

Server::Server(const int port): socket_(port), event_loop_(1000) {
    auto serverEventHandler = [this](const int fd, const uint32_t events) {
        HandleServerEvent(fd, events);
    };
    event_loop_.AddFd(socket_.GetServerFd(), EPOLLIN, serverEventHandler);
}

void Server::HandleServerEvent(const int server_fd, const uint32_t events) {
    if (events & EPOLLIN) {
        const int client_fd = Socket::AcceptConnection(server_fd);
        if (client_fd < 0) {
            std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
            return;
        }
        const auto client = std::make_shared<Client>();
        client->fd = client_fd;
        clients_[client_fd] = client;

        auto clientEventHandler = [this, client](const int fd, const uint32_t events) {
            HandleClientEvent(client, events);
        };
        event_loop_.AddFd(client_fd, EPOLLIN | EPOLLERR | EPOLLHUP, clientEventHandler);
    }
}

void Server::HandleClientEvent(const std::shared_ptr<Client> &client, const uint32_t events) {
    if (events & (EPOLLERR | EPOLLHUP)) {
        CloseClient(client);
        return;
    }

    if (events & EPOLLIN) {
        char buffer[1024]{};
        const ssize_t n = read(client->fd, buffer, sizeof(buffer));

        if (n <= 0) {
            if (n == 0 || errno != EAGAIN) {
                CloseClient(client);
                return;
            }
        }

        std::string input(buffer, n);

        boost::asio::post(pool_, [this, client, input] {
            {
                const std::string response = CommandExecutor::Executor(input);
                std::lock_guard lock(client->mutex);
                client->write_buffer += response;
            }
            // Schedule sending the response in the main thread
            boost::asio::post(event_loop_.GetIoContext(), [this, client] {
                TryWrite(client);
            });
        });
    }

    if (events & EPOLLOUT) {
        TryWrite(client);
    }
}

void Server::TryWrite(const std::shared_ptr<Client> &client) {

    std::lock_guard lock(client->mutex);

    if (client->closed) {
        return;
    }

    if (client->write_buffer.empty()) {
        if (!event_loop_.UpdateFd(client->fd, EPOLLIN | EPOLLERR | EPOLLHUP)) {
            CloseClient(client);
        }
        return;
    }

    ssize_t n = write(client->fd, client->write_buffer.data(), client->write_buffer.size());
    if (n > 0) {
        client->write_buffer.erase(0, n);
    } else if (n == 0 || errno != EAGAIN) {
        CloseClient(client);
        return;
    }

    uint32_t events = EPOLLIN | EPOLLERR | EPOLLHUP;

    if (!client->write_buffer.empty()) {
        events |= EPOLLOUT;
    }

    if (!event_loop_.UpdateFd(client->fd, events)) {
        CloseClient(client);
    }
}

void Server::CloseClient(const std::shared_ptr<Client> &client) {
    if (client->closed) {
        return;
    }
    event_loop_.RemoveFd(client->fd);
    clients_.erase(client->fd);

    if (client->fd >= 0) {
        close(client->fd);
        client->fd = -1;
    }

    client->closed = true;

    std::cerr << "Closed client with fd " << client->fd << std::endl;
}
