#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include <functional>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <vector>

class EventLoop {
public:
    using EventCallback = std::function<void(int fd, uint32_t events)>;

    explicit EventLoop(int max_events);

    void AddFd(int fd, uint32_t events, const EventCallback &callback);

    void RemoveFd(int fd) const;

    void Run();

    ~EventLoop();

private:
    int epoll_fd_{-1};
    int max_events_{1000};
    std::vector<epoll_event> events_{};
    std::unordered_map<int, EventCallback> callbacks_{};

    static void SetNonBlocking(int fd);
};


#endif //EVENT_LOOP_H
