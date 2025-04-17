#pragma once

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>

struct FieldValuePair {
    std::string key;
    std::string value;

    FieldValuePair(std::string key, std::string value) noexcept
        : key(std::move(key)), value(std::move(value)) {}
};

struct StreamEntry {
    std::string entry_id;
    std::vector<FieldValuePair> fields;

    StreamEntry() noexcept = default;

    StreamEntry(std::string entry_id, std::string key, std::string value) noexcept
        : entry_id(std::move(entry_id)) {
        fields.emplace_back(std::move(key), std::move(value));
    }
};

class RedisStream {
public:
    RedisStream(const RedisStream&) = delete;
    RedisStream& operator=(const RedisStream&) = delete;
    RedisStream(RedisStream&&) = delete;
    RedisStream& operator=(RedisStream&&) = delete;

    static RedisStream &GetInstance() {
        static RedisStream instance;
        return instance;
    }

    std::string Add(const std::string& stream_key, const std::string &entry_id, const std::string &key, const std::string &value);
    bool ExistStreamKey(const std::string& stream_key) const;

private:
    RedisStream() = default;
    ~RedisStream() = default;
    bool Exists(const std::string &stream_key, const std::string &entry_id);
    std::unordered_map<std::string, StreamEntry> stream_entries_{};
};
