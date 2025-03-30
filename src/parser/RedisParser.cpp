#include "RedisParser.h"
#include <ostream>

#include "../utils/Utils.h"

RedisDatabase RedisParser::redis_database_;

std::string RedisParser::HandleCommand(const std::string &input) {
    if (input.empty() || !Utils::FindCRLF(input).has_value()) {
        return "-ERR Unknown command\r\n";
    }
    switch (input[0]) {
        case '+': return HandleSimpleString(input);
        case '-': return HandleError(input);
        case ':': return HandleInteger(input);
        case '$': return HandleBulkString(input);
        case '*': return HandleArray(input);
        default: return "-ERR Unknown command type\r\n";
    }
}

std::string RedisParser::HandleSimpleString(const std::string &input) {
    const auto pos = Utils::FindCRLF(input);
    const std::string data = input.substr(1, *pos - 1);

    return (Utils::ToLowerCase(data) == "ping") ? "+PONG\r\n" : "+" + data + "\r\n";
}

std::string RedisParser::HandleError(const std::string &input) {
    const auto pos = Utils::FindCRLF(input);
    const std::string data = input.substr(1, *pos - 1);
    return "-" + data + "\r\n";
}

std::string RedisParser::HandleInteger(const std::string &input) {
    const auto pos = Utils::FindCRLF(input);
    const std::string data = input.substr(1, *pos - 1);
    if (!Utils::IsNumeric(data)) {
        return "-ERR Invalid integer number\r\n";
    }
    return ":" + data + "\r\n";
}

std::string RedisParser::HandleBulkString(const std::string &input) {
    const auto pos_crlf = Utils::FindCRLF(input);

    if (!pos_crlf.has_value()) {
        return "-ERR Invalid bulk string\r\n";
    }

    const auto prefix_length_str = input.substr(1, *pos_crlf - 1);

    if (!Utils::IsNumeric(prefix_length_str)) {
        return "-ERR Invalid bulk string\r\n";
    }

    const int prefix_length = std::stoi(prefix_length_str);

    if (prefix_length == -1) {
        return "$-1\r\n";
    }

    const size_t content_start = *pos_crlf + 2;
    size_t content_end = content_start + prefix_length;

    if (content_end + 2 > input.size() || input.substr(content_end, 2) != "\r\n") {
        return "-ERR Invalid bulk string\r\n";
    }

    const std::string content = input.substr(content_start, prefix_length);

    return "$" + std::to_string(prefix_length) + "\r\n" + content + "\r\n";
}


std::vector<std::string> RedisParser::parseTokens(const std::string &input) {
    std::vector<std::string> tokens;
    size_t start = 0;
    while (start < input.size()) {
        const size_t pos = input.find("\r\n", start);
        if (pos == std::string::npos) break;
        tokens.push_back(input.substr(start, pos - start));
        start = pos + 2;
    }
    return tokens;
}

std::string RedisParser::HandleArray(const std::string &input) {
    const auto pos_crlf = Utils::FindCRLF(input);
    if (!pos_crlf.has_value()) {
        return "-ERR Invalid array\r\n";
    }

    const auto array_length_str = input.substr(1, *pos_crlf - 1);

    if (!Utils::IsNumeric(array_length_str)) {
        return "-ERR Invalid array length\r\n";
    }

    const int array_length = std::stoi(array_length_str);
    if (array_length <= 0) {
        return "-ERR Invalid array length\r\n";
    }

    const std::vector<std::string> tokens = parseTokens(input);

    if (array_length == 1) {
        if ("ping" == Utils::ToLowerCase(tokens[2])) {
            return "+PONG\r\n";
        }
        return "+" + tokens[2] + "\r\n";
    }

    const std::string command = Utils::ToLowerCase(tokens[2]);

    if ("echo" == command && tokens.size() == 5) {
        return tokens[3] + "\r\n" + tokens[4] + "\r\n";
    }

    if ("set" == command && tokens.size() == 7) {
        const std::string key = tokens[4];
        const std::string value = tokens[6];
        redis_database_.set(key, value);
        return "+OK\r\n";
    }

    if ("get" == command && tokens.size() == 5) {
        const std::string key = tokens[4];
        const std::string value = redis_database_.get(key);
        if ("nil" == value) {
            return "$-1\r\n";
        }
        return "$" + std::to_string(value.length()) + "\r\n" + value + "\r\n";
    }

    return "-ERR Invalid command\r\n";
}
