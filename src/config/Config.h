#pragma once

#include <string>
#include <unordered_map>

class Config {
public:
    Config(const Config &) = delete;
    void operator=(const Config &) = delete;

    static Config &GetInstance() {
        static Config instance;
        return instance;
    }

    void Set(const std::string &key, const std::string &value);

    std::string Get(const std::string &key);

    std::string GetRdbUrl();

private:
    Config();

    std::unordered_map<std::string, std::string> config_{};
};