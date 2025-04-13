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

    static  std::string ToError(const std::string &message);

    static std::string OkResponse();

    static std::string NilResponse();

private:
    static RespEntry ParseEntry(const std::string &input, size_t &pos);
};
