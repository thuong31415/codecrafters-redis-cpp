#pragma once

#include <string>
#include <unordered_map>
#include <vector>

class RedisDatabase {
public:
    RedisDatabase(const RedisDatabase&) = delete;
    RedisDatabase& operator=(const RedisDatabase&) = delete;
    RedisDatabase(RedisDatabase&&) = delete;
    RedisDatabase& operator=(RedisDatabase&&) = delete;

    static RedisDatabase &GetInstance() {
        static RedisDatabase instance;
        return instance;
    }

    void Set(const std::string &key, const std::string &value);

    void Set(const std::string &key, const std::string &value, int64_t ttl);

    std::string Get(const std::string &key);

    void Del(const std::string &key);

    std::vector<std::string> GetAllKeys() const;

private:
    RedisDatabase() = default;
    ~RedisDatabase() = default;

    std::unordered_map<std::string, std::pair<std::string, int64_t> > store_{};
};