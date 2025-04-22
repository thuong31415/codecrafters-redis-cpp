#include "EventLoop.h"

#include <cstring>
#include <stdexcept>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <iostream>
#include <mutex>
#include <sys/epoll.h>

EventLoop::EventLoop(const int max_events): max_events_(max_events) {
    epoll_fd_ = epoll_create1(EPOLL_CLOEXEC);
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

    std::unique_lock lock(callbacks_mutex_);
    callbacks_[fd] = callback;;
}

bool EventLoop::UpdateFd(const int fd, const uint32_t events) const {
    epoll_event event{};
    event.events = events;
    event.data.fd = fd;
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &event) < 0) {
        std::cerr << "epoll_ctl EPOLL_CTL_MOD failed for fd " << fd << ": " << strerror(errno) << std::endl;
        return false;
    }
    return true;
}


void EventLoop::RemoveFd(const int fd) {
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) < 0 && errno != EBADF) {
        std::cerr << "Warning: Failed to remove fd from epoll: " << strerror(errno) << std::endl;
    }

    shutdown(fd, SHUT_RDWR);

    std::unique_lock lock(callbacks_mutex_);
    callbacks_.erase(fd);
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


boost::asio::io_context &EventLoop::GetIoContext() {
    return io_context_;
}

void EventLoop::Run() {
    boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard(io_context_.get_executor());

    while (true) {
        const int n = epoll_wait(epoll_fd_, events_.data(), max_events_, 10);

        if (n < 0) {
            if (errno == EINTR) continue;
            throw std::runtime_error("epoll_wait failed");
        }

        for (int i = 0; i < n; i++) {
            int fd = events_[i].data.fd;
            const uint32_t event_flags = events_[i].events;

            EventCallback callback; {
                std::shared_lock lock(callbacks_mutex_);
                if (auto it = callbacks_.find(fd); it != callbacks_.end()) {
                    callback = it->second;
                }
            }
            if (callback) {
                callback(fd, event_flags);
            }
        }
        io_context_.poll();
    }
}
