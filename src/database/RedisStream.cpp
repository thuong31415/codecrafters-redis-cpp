#include "RedisStream.h"
#include "../parser/RespParser.h"
#include "../utils/Utils.h"

std::string RedisStream::Add(const std::string &stream_key,
                             const std::string &entry_id,
                             const std::string &key,
                             const std::string &value) {
    int64_t new_timestamp{};
    int64_t new_sequence{0};

    auto add_new_stream = [&]()-> std::string {
        const StreamEntry new_entry{new_timestamp, new_sequence, key, value};
        stream_entries_[stream_key].push_back(new_entry);
        const std::string new_entry_id = std::to_string(new_timestamp) + "-" + std::to_string(new_sequence);
        return RespParser::ToBulkString(new_entry_id);
    };

    if (entry_id == "*") {
        new_timestamp = Utils::GetCurrentTimestamp();
        new_sequence = 0;
        return add_new_stream();
    }

    const auto pos = entry_id.find('-');
    if (pos == std::string::npos) {
        return RespParser::ToError("ERR wrong format of entry_id");
    }

    const std::string timestamp_str = entry_id.substr(0, pos);
    const std::string sequence_str = entry_id.substr(pos + 1);

    if (!Utils::IsNumeric(timestamp_str)) {
        return RespParser::ToError("ERR wrong number of arguments for 'xadd' command");
    }

    new_timestamp = std::stoll(timestamp_str);

    if (!stream_entries_.contains(stream_key)) {
        if (sequence_str == "*") {
            new_sequence = new_timestamp == 0 ? 1 : 0;
        } else if (Utils::IsNumeric(sequence_str)) {
            new_sequence = std::stoll(sequence_str);
        } else {
            return RespParser::ToError("ERR wrong number of arguments for 'xadd' command");
        }
        return add_new_stream();
    }

    const std::vector<StreamEntry> &entries = stream_entries_[stream_key];
    const StreamEntry last_entry = entries.back();

    if (sequence_str == "*") {
        new_sequence = last_entry.timestamp == new_timestamp ? last_entry.sequence + 1 : 0;
    } else if (Utils::IsNumeric(sequence_str)) {
        new_sequence = std::stoll(sequence_str);
    } else {
        return RespParser::ToError("ERR wrong number of arguments for 'xadd' command");
    }

    if (new_timestamp == 0 && new_sequence == 0) {
        return RespParser::ToError("ERR The ID specified in XADD must be greater than 0-0");
    }

    const bool is_valid_sequence = new_timestamp > last_entry.timestamp
                                   || (new_timestamp == last_entry.timestamp && new_sequence > last_entry.sequence);

    if (!is_valid_sequence) {
        return RespParser::ToError("ERR The ID specified in XADD is equal or smaller than the target stream top item");
    }
    return add_new_stream();
}

bool RedisStream::ExistStreamKey(const std::string &stream_key) const {
    return stream_entries_.contains(stream_key);
}
