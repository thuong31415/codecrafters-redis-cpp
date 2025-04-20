#pragma once
#include <string>
#include <variant>
#include <vector>

enum class RespType {
    simple_string,
    bulk_string,
    array,
    error,
    integer,
    null,
    unknown
};

struct RespEntry {
    RespType type;
    std::variant<std::string, int64_t, std::vector<RespEntry>, std::monostate> value;

    RespEntry(const RespType t, std::string v) : type(t), value(std::move(v)) {}
    RespEntry(const RespType t, int64_t v) : type(t), value(v) {}
    RespEntry(const RespType t, std::vector<RespEntry> v) : type(t), value(std::move(v)) {}
    RespEntry(const RespType t, std::monostate) : type(t), value(std::monostate{}) {}

    std::string AsString() const {
        return std::get<std::string>(value);
    }
};
