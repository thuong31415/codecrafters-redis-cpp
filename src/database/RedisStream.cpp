#include "RedisStream.h"
#include "../parser/RespParser.h"

std::string RedisStream::Add(const std::string &stream_key,
                             const std::string &entry_id,
                             const std::string &key,
                             const std::string &value) {
    if (Exists(stream_key, entry_id)) {
        return RespParser::ToError("ERR The ID specified in XADD is equal or smaller than the target stream top item");
    }

    const StreamEntry new_entry{entry_id, key, value};
    stream_entries_[stream_key] = new_entry;

    return RespParser::ToBulkString(entry_id);
}

bool RedisStream::Exists(const std::string &stream_key, const std::string &entry_id) {
    if (stream_entries_.contains(stream_key)) {
        return false;
    }
    const StreamEntry stream_entry = stream_entries_[stream_key];

    return stream_entry.entry_id == entry_id;
}

bool RedisStream::ExistStreamKey(const std::string &stream_key) const {
    return stream_entries_.contains(stream_key);
}

