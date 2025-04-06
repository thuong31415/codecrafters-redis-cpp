#ifndef SERVER_H
#define SERVER_H

#include <RedisParser.h>

#include "../network/Socket.h"
#include  "../network/EventLoop.h"


class Server {
public:
    explicit Server(int port);

    void Start();

private:
    Socket socket_;
    EventLoop event_loop_;
    RedisParser redis_parser_;

    void HandleServerEvent(int server_fd, uint32_t events);

    void HandleClientEvent(int client_fd, uint32_t events);
};


#endif //SERVER_H
