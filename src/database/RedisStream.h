#pragma once

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <stack>

struct FieldValuePair {
    std::string key;
    std::string value;

    FieldValuePair(std::string key, std::string value) noexcept
        : key(std::move(key)), value(std::move(value)) {
    }
};

struct StreamEntry {
    int64_t timestamp{};
    int64_t sequence{};
    std::vector<FieldValuePair> fields;

    StreamEntry() noexcept = default;

    StreamEntry(const int64_t timestamp, const int64_t sequence, std::string key, std::string value)
        : timestamp(timestamp), sequence(sequence) {
        fields.emplace_back(std::move(key), std::move(value));
    }
};

class RedisStream {
public:
    RedisStream(const RedisStream &) = delete;

    RedisStream &operator=(const RedisStream &) = delete;

    RedisStream(RedisStream &&) = delete;

    RedisStream &operator=(RedisStream &&) = delete;

    static RedisStream &GetInstance() {
        static RedisStream instance;
        return instance;
    }

    std::string Add(const std::string &stream_key, const std::string &entry_id, const std::string &key,
                    const std::string &value);

    bool ExistStreamKey(const std::string &stream_key) const;

private:
    RedisStream() = default;
    ~RedisStream() = default;

    std::unordered_map<std::string, std::vector<StreamEntry> > stream_entries_{};
};
