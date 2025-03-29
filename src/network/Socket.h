#ifndef SOCKET_H
#define SOCKET_H
#include <cstdint>

class Socket {
public:
    explicit Socket(uint16_t port);

    ~Socket();

    [[nodiscard]] static int AcceptConnection(int server_fd) ;

    [[nodiscard]] int GetServerFd() const;

    [[nodiscard]] uint16_t GetPort() const;

private:
    int server_fd_;
    uint16_t port_;
};


#endif //SOCKET_H
