#pragma once

#include <functional>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <vector>
#include <shared_mutex>
#include <boost/asio/io_context.hpp>

class EventLoop {
public:

    // Non-copyable and non-movable
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;
    EventLoop(EventLoop&&) = delete;
    EventLoop& operator=(EventLoop&&) = delete;

    using EventCallback = std::function<void(int fd, uint32_t events)>;

    explicit EventLoop(int max_events);

    void AddFd(int fd, uint32_t events, const EventCallback &callback);

    bool UpdateFd(int fd, uint32_t events) const;

    void RemoveFd(int fd);

    void Run();

    boost::asio::io_context& GetIoContext();

    ~EventLoop();


private:
    int epoll_fd_{-1};
    int max_events_{1000};

    std::vector<epoll_event> events_{};
    std::shared_mutex callbacks_mutex_;
    std::unordered_map<int, EventCallback> callbacks_{};
    boost::asio::io_context io_context_;

    static void SetNonBlocking(int fd);
};