#include "RespParser.h"

#include <ostream>
#include <stdexcept>

RespEntry RespParser::ParseCommand(const std::string &input) {
    size_t pos = 0;
    return ParseEntry(input, pos);
}

RespEntry RespParser::ParseEntry(const std::string &input, size_t &pos) {
    if (pos >= input.size()) {
        throw std::runtime_error("Invalid RESP: unexpected end of input");
    }

    auto read_line = [&]()-> std::string {
        const size_t end = input.find("\r\n", pos);
        if (end == std::string::npos) {
            throw std::runtime_error("Invalid RESP: missing CRLF");
        }
        std::string line = input.substr(pos, end - pos);
        pos = end + 2;
        return line;
    };

    auto parse_integer = [&]()-> int64_t  {
        const std::string line = read_line();
        size_t idx = 0;
        const int64_t value = std::stoll(line, &idx);
        if (idx != line.size()) {
            throw std::runtime_error("Invalid RESP integer format");
        }
        return value;
    };

    const char type = input[pos];
    pos++;

    switch (type) {
        case '+': return {RespType::simple_string, read_line()};
        case ':': return {RespType::integer, parse_integer()};
        case '-': return {RespType::error, read_line()};
        case '$': {
            const int64_t len = parse_integer();
            if (len == -1) return {RespType::null, std::monostate{}};
            if (pos + len > input.size()) {
                throw std::runtime_error("Invalid RESP: bulk string length exceeds input");
            }
            std::string value = input.substr(pos, len);
            pos += len;

            if (pos + 2 > input.size() || input[pos] != '\r' || input[pos + 1] != '\n') {
                throw std::runtime_error("Invalid RESP: missing CRLF after bulk string");
            }
            pos += 2;
            return {RespType::bulk_string, std::move(value)};
        }
        case '*': {
            const int64_t count = parse_integer();
            if (count == -1) return {RespType::null, std::monostate{}};
            std::vector<RespEntry> elements;
            elements.reserve(count);

            for (int i = 0; i < count; ++i) {
                elements.push_back(ParseEntry(input, pos));
            }
            return {RespType::array, std::move(elements)};
        }
        default:
            throw std::runtime_error(std::string("Unknown RESP type: ") + type);
    }
}

std::string RespParser::ToSimpleString(const std::string &val) {
    return "+" + val + std::string(CRLF);
}

std::string RespParser::ToBulkString(const std::string &val) {
    return "$" + std::to_string(val.length()) + std::string(CRLF) + val + std::string(CRLF);
}

std::string RespParser::ToArrayString(const std::vector<std::string> &tokens) {
    std::string result = "*" + std::to_string(tokens.size()) + std::string(CRLF);
    for (const auto &token: tokens) {
        result += ToBulkString(token);
    }
    return result;
}

std::string RespParser::ToError(const std::string &message) {
    return "-" + message + "\r\n";
}

std::string RespParser::OkResponse() {
    return ToSimpleString("OK");
}

std::string RespParser::NilResponse() {
    return "$-1\r\n";
}
