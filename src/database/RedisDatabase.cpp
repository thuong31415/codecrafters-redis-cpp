#include "RedisDatabase.h"

#include <Utils.h>

RedisDatabase::RedisDatabase() = default;

RedisDatabase::~RedisDatabase() {
    store_.clear();
}

void RedisDatabase::set(const std::string &key, const std::string &value, const int64_t ttl) {
    int64_t expiration = ttl > 0 ? Utils::GetCurrentTimestamp() + ttl : -1;
    store_[key] = {value, expiration};
}

std::string RedisDatabase::get(const std::string &key) {
    const auto it = store_.find(key);

    if (it == store_.end()) {
        return "nil";
    }

    if (it->second.second == -1 ) {
        return it->second.first;
    }

    if (Utils::GetCurrentTimestamp() < it->second.second) {
        return it->second.first;
    }
    store_.erase(it);
    return "nil";
}

void RedisDatabase::del(const std::string &key) {
    const auto it = store_.find(key);
    if (it == store_.end()) {
        return;
    }
    store_.erase(it);
}
