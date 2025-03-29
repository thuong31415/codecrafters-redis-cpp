#include "EventLoop.h"
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/epoll.h>

EventLoop::EventLoop(const int max_events): max_events_(max_events) {
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ == -1) {
        throw std::runtime_error("Failed to create epoll instance");
    }
    events_.resize(max_events);
}

EventLoop::~EventLoop() {
    if (epoll_fd_ != -1) {
        close(epoll_fd_);
    }
}

void EventLoop::AddFd(const int fd, const uint32_t events, const EventCallback &callback) {
    SetNonBlocking(fd);
    epoll_event event{};
    event.events = events;
    event.data.fd = fd;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &event) < 0) {
        throw std::runtime_error("Failed to add fd to epoll");
    }
    callbacks_[fd] = callback;;
}

void EventLoop::RemoveFd(const int fd) const {
    epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
}

void EventLoop::SetNonBlocking(const int fd) {
    const int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        throw std::runtime_error("Failed to get fd flags");
    }
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        throw std::runtime_error("Failed to set non-blocking");
    }
}

void EventLoop::Run() {
    while (true) {
        const int n = epoll_wait(epoll_fd_, events_.data(), max_events_, -1);
        if (n < 0) {
            if (errno == EINTR) continue;
            throw std::runtime_error("epoll_wait failed");
        }

        for (int i = 0; i < n; i++) {
            int fd = events_[i].data.fd;
            const uint32_t event_flags = events_[i].events;
            if (auto it = callbacks_.find(fd); it != callbacks_.end()) {
                it->second(fd, event_flags);
            }
        }
    }
}
