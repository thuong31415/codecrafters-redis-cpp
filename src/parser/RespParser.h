#pragma once

#include <string>
#include <string_view>
#include <vector>
#include "RespTypes.h"

inline constexpr std::string_view CRLF = "\r\n";

class RespParser {
public:
    RespParser() = default;

    static RespEntry ParseCommand(const std::string &input);

    static std::string ToSimpleString(const std::string &val);

    static std::string ToBulkString(const std::string &val);

    static std::string ToArrayString(const std::vector<std::string> &tokens);

    static std::string Error(const std::string &message);

    static std::string Ok();

    static std::string Nil();

    static std::string Empty();

    template<typename... Args>
    static std::string ToArrayString(const std::string &first, const Args &... args) {
        const std::vector<std::string> tokens = {first, args...};
        std::string result = "*" + std::to_string(tokens.size()) + "\r\n";
        for (const auto &token: tokens) {
            result += ToBulkString(token);
        }
        return result;
    }

private:
    static RespEntry ParseEntry(const std::string &input, size_t &pos);
};
