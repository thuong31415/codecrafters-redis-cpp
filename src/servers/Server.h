#pragma once

#include <memory>

#include "../network/Socket.h"
#include  "../network/EventLoop.h"
#include "../command/CommandExecutor.h"
#include "../network/Client.h"
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/post.hpp>


class Server {
public:
    explicit Server(int port);

    void Start();

private:
    Socket socket_;
    EventLoop event_loop_;
    std::unordered_map<int, std::shared_ptr<Client> > clients_;
    boost::asio::thread_pool pool_{10};

    void HandleServerEvent(int server_fd, uint32_t events);

    void HandleClientEvent(const std::shared_ptr<Client> &client, uint32_t events);

    void TryWrite(const std::shared_ptr<Client> &client);

    void CloseClient(const std::shared_ptr<Client> &client);
};
