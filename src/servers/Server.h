#ifndef SERVER_H
#define SERVER_H

#include "../network/Socket.h"
#include  "../network/EventLoop.h"


class Server {
public:
    explicit Server(int port);

    void Start();

private:
    Socket socket_;
    EventLoop event_loop_;

    void HandleServerEvent(int server_fd, uint32_t events);

    static void HandleClientEvent(int client_fd, uint32_t events);
};


#endif //SERVER_H
