#pragma once
#include<string>

struct Client {
    int fd{-1};
    std::string read_buffer;
    std::string write_buffer;
    bool closed{false};
    std::mutex mutex;
};
