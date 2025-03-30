#include "RedisDatabase.h"

RedisDatabase::RedisDatabase() {
}

RedisDatabase::~RedisDatabase() {
    store_.clear();
}

void RedisDatabase::set(const std::string &key, const std::string &value) {
    store_[key] = value;
}

std::string RedisDatabase::get(const std::string &key) {
    return store_.contains(key) ? store_[key] : std::string("nil");
}

void RedisDatabase::del(const std::string &key) {
    const auto it = store_.find(key);
    if (it == store_.end()) {
        return;
    }
    store_.erase(it);
}
