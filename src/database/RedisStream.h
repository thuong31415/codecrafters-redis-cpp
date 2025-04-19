#pragma once

#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <stdexcept>

#include "RespParser.h"

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
    std::vector<FieldValuePair> fields{};

    StreamEntry() noexcept = default;

    StreamEntry(const int64_t timestamp, const int64_t sequence, std::string key, std::string value)
        : timestamp(timestamp), sequence(sequence) {
        fields.emplace_back(std::move(key), std::move(value));
    }

    [[nodiscard]] std::string ToResp() const {
        std::string resp = "*" + std::to_string(2) + "\r\n";
        const std::string entry_id = std::to_string(timestamp) + "-" + std::to_string(sequence);
        resp += RespParser::ToBulkString(entry_id);
        for (const auto &fild: fields) {
            resp += RespParser::ToArrayString(fild.key, fild.value);
        }
        return resp;
    }

    explicit StreamEntry(const std::string &entry_id) {
        const auto pos = entry_id.find('-');
        if (pos == std::string::npos) {
            throw std::runtime_error("Failed to create epoll instance");
        }
        timestamp = std::stoll(entry_id.substr(0, pos));
        sequence = std::stoll(entry_id.substr(pos + 1));
    }

    bool operator<=(const StreamEntry &other) const {
        if (timestamp == other.timestamp && sequence == other.sequence) {
            return true;
        }
        if (timestamp != other.timestamp) {
            return timestamp < other.timestamp;
        }
        return sequence < other.sequence;
    }

    bool operator>=(const StreamEntry &other) const {
        if (timestamp == other.timestamp && sequence == other.sequence) {
            return true;
        }
        if (timestamp != other.timestamp) {
            return timestamp > other.timestamp;
        }
        return sequence > other.sequence;
    }

    bool operator==(const StreamEntry &other) const {
        return timestamp == other.timestamp && sequence == other.sequence;
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

    std::vector<StreamEntry> GetByStreamKey(const std::string &stream_key) const;

private:
    RedisStream() = default;

    ~RedisStream() = default;

    std::unordered_map<std::string, std::vector<StreamEntry> > stream_entries_{};
};
