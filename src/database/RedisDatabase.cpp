#include "RedisDatabase.h"

#include <Utils.h>


void RedisDatabase::Set(const std::string &key, const std::string &value, const int64_t ttl) {
    int64_t expiration = ttl > 0 ? Utils::GetCurrentTimestamp() + ttl : -1;
    store_[key] = {value, expiration};
}

void RedisDatabase::Set(const std::string &key, const std::string &value) {
    Set(key, value, 0);
}

std::string RedisDatabase::Get(const std::string &key) {
    const auto it = store_.find(key);

    if (it == store_.end()) {
        return "nil";
    }

    if (it->second.second == -1) {
        return it->second.first;
    }

    if (Utils::GetCurrentTimestamp() < it->second.second) {
        return it->second.first;
    }
    store_.erase(it);
    return "nil";
}

void RedisDatabase::Del(const std::string &key) {
    const auto it = store_.find(key);
    if (it == store_.end()) {
        return;
    }
    store_.erase(it);
}

std::vector<std::string> RedisDatabase::GetAllKeys() const {
    const auto now = Utils::GetCurrentTimestamp();

    std::vector<std::string> keys;
    keys.reserve(store_.size());

    for (const auto& [key, value_pair] : store_) {
        if (value_pair.second == -1 || now <= value_pair.second) {
            keys.push_back(key);
        }
    }
    return keys;
}
