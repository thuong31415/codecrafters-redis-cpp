#include "RedisStream.h"
#include "../parser/RespParser.h"
#include "../utils/Utils.h"

std::string RedisStream::Add(const std::string &stream_key,
                             const std::string &entry_id,
                             const std::string &key,
                             const std::string &value) {

    const auto pos = entry_id.find('-');
    if (pos == std::string::npos) {
        return RespParser::ToError("ERR wrong format of entry_id");
    }

    const std::string timestamp_str = entry_id.substr(0, pos);
    const std::string sequence_str = entry_id.substr(pos + 1);

    if (!Utils::IsNumeric(timestamp_str) || !Utils::IsNumeric(sequence_str)) {
        return RespParser::ToError("ERR wrong number of arguments for 'xadd' command");
    }

    const int64_t new_timestamp = std::stoll(timestamp_str);
    const int64_t new_sequence = std::stoll(sequence_str);

    if (new_timestamp == 0 && new_sequence == 0) {
        return RespParser::ToError("ERR The ID specified in XADD must be greater than 0-0");
    }

    auto add_stream = [&]()-> std::string {
        const StreamEntry new_entry{new_timestamp, new_sequence, key, value};
        stream_entries_[stream_key].push_back(new_entry);
        return RespParser::ToBulkString(entry_id);
    };

    if (!stream_entries_.contains(stream_key)) {
        return add_stream();
    }

    const std::vector<StreamEntry> &entries = stream_entries_[stream_key];
    const StreamEntry last_entry = entries.back();

    const bool is_valid_sequence = new_timestamp > last_entry.timestamp
                                   || (new_timestamp == last_entry.timestamp && new_sequence > last_entry.sequence);

    if (!is_valid_sequence) {
        return RespParser::ToError("ERR The ID specified in XADD is equal or smaller than the target stream top item");
    }
    return add_stream();
}

bool RedisStream::ExistStreamKey(const std::string &stream_key) const {
    return stream_entries_.contains(stream_key);
}
