#include "RedisParser.h"

#include <iostream>

#include "../config/Config.h"
#include "../utils/Utils.h"

namespace {
    const std::string OK_RESPONSE = "+OK\r\n";
    const std::string PONG_RESPONSE = "+PONG\r\n";
    const std::string NIL_RESPONSE = "$-1\r\n";
    const std::string INVALID_COMMAND_ERROR = "-ERR Unknown command\r\n";
    const std::string INVALID_ARRAY_ERROR = "-ERR Invalid array\r\n";
    const std::string INVALID_BULK_STRING_ERROR = "-ERR Invalid bulk string\r\n";
    const std::string INVALID_INTEGER_ERROR = "-ERR Invalid integer number\r\n";
    const std::string SYNTAX_ERROR = "-ERR syntax error\r\n";
}


std::string RedisParser::HandleCommand(const std::string &input) {
    if (input.empty() || !Utils::FindCRLF(input).has_value()) {
        return INVALID_COMMAND_ERROR;
    }
    switch (input[0]) {
        case '+': return HandleSimpleString(input);
        case '-': return HandleError(input);
        case ':': return HandleInteger(input);
        case '$': return HandleBulkString(input);
        case '*': return HandleArrayCommand(input);
        default: return INVALID_COMMAND_ERROR;
    }
}

std::string RedisParser::HandleSimpleString(const std::string &input) {
    const auto pos = Utils::FindCRLF(input);
    const std::string data = input.substr(1, *pos - 1);

    return Utils::ToLowerCase(data) == "ping" ? PONG_RESPONSE : "+" + data + "\r\n";
}

std::string RedisParser::HandleError(const std::string &input) {
    const auto pos = Utils::FindCRLF(input);
    const std::string data = input.substr(1, *pos - 1);
    return "-" + data + "\r\n";
}

std::string RedisParser::HandleInteger(const std::string &input) {
    const auto pos = Utils::FindCRLF(input);
    const std::string data = input.substr(1, *pos - 1);

    return !Utils::IsNumeric(data) ? INVALID_INTEGER_ERROR : ":" + data + "\r\n";
}

std::string RedisParser::HandleBulkString(const std::string &input) {
    const auto pos_crlf = Utils::FindCRLF(input);

    if (!pos_crlf.has_value()) {
        return INVALID_BULK_STRING_ERROR;
    }

    const auto prefix_length_str = input.substr(1, *pos_crlf - 1);

    if (!Utils::IsNumeric(prefix_length_str)) {
        return INVALID_BULK_STRING_ERROR;
    }

    const int prefix_length = std::stoi(prefix_length_str);

    if (prefix_length == -1) {
        return NIL_RESPONSE;
    }

    const size_t content_start = *pos_crlf + 2;
    size_t content_end = content_start + prefix_length;

    if (content_end + 2 > input.size() || input.substr(content_end, 2) != "\r\n") {
        return INVALID_BULK_STRING_ERROR;
    }

    const std::string content = input.substr(content_start, prefix_length);

    return "$" + std::to_string(prefix_length) + "\r\n" + content + "\r\n";
}

std::string RedisParser::HandleArrayCommand(const std::string &input) {
    const auto pos_crlf = Utils::FindCRLF(input);
    if (!pos_crlf.has_value()) {
        return INVALID_ARRAY_ERROR;
    }

    const auto array_length_str = input.substr(1, *pos_crlf - 1);

    if (!Utils::IsNumeric(array_length_str)) {
        return INVALID_ARRAY_ERROR;
    }

    if (const int array_length = std::stoi(array_length_str); array_length <= 0) {
        return INVALID_ARRAY_ERROR;
    }

    const std::vector<std::string> tokens = ParseTokens(input);

    if (tokens.size() == 2) {
        return tokens[1] + "\r\n";
    }

    const std::string command = Utils::ToLowerCase(tokens[2]);

    if ("ping" == command) {
        return HandlePingCommand(tokens);
    }

    if ("echo" == command) {
        return HandleEchoCommand(tokens);
    }

    if ("set" == command) {
        return HandleSetCommand(tokens);
    }

    if ("get" == command) {
        return HandleGetCommand(tokens);
    }

    if ("config" == command) {
        return HandleConfigCommand(tokens);
    }

    if ("keys" == command) {
        return HandleKeysCommand(tokens);
    }

    if ("type" == command) {
        return HandleTypeCommand(tokens);
    }

    return SYNTAX_ERROR;
}

std::string RedisParser::HandlePingCommand(const std::vector<std::string> &tokens) {
    return tokens.size() == 3 ? PONG_RESPONSE : INVALID_COMMAND_ERROR;
}

std::string RedisParser::HandleEchoCommand(const std::vector<std::string> &tokens) {
    return tokens.size() == 5 ? tokens[3] + "\r\n" + tokens[4] + "\r\n" : INVALID_COMMAND_ERROR;
}

std::string RedisParser::HandleSetCommand(const std::vector<std::string> &tokens) {
    if (tokens.size() != 7 && tokens.size() != 11) {
        return INVALID_COMMAND_ERROR;
    }

    const std::string &key = tokens[4];
    const std::string &value = tokens[6];

    if (tokens.size() == 7) {
        redis_database_.set(key, value, 0);
        return OK_RESPONSE;
    }

    if (Utils::ToLowerCase(tokens[8]) == "px" && Utils::IsNumeric(tokens[10])) {
        redis_database_.set(key, value, std::stoi(tokens[10]));
        return OK_RESPONSE;
    }

    return SYNTAX_ERROR;
}


std::string RedisParser::HandleGetCommand(const std::vector<std::string> &tokens) {

    const std::string &key = tokens[4];
    std::string value = redis_database_.get(key);

    if ("nil" != value) {
        return ToBulkString(value);
    }

    Config &config = Config::getInstance();
    const std::string path = config.get("dir") + "/" + config.get("dbfilename");
    rdb_reader_.Process(path);

    const auto &data = rdb_reader_.GetData();

    const auto it = data.find(key);

    if (it == data.end()) {
        return NIL_RESPONSE;
    }

    value = it->second.first;

    if (const int64_t ttl = it->second.second; ttl > 0 && ttl <= Utils::GetCurrentTimestamp()) {
        return NIL_RESPONSE;
    }
    return ToBulkString(value);
}

std::string RedisParser::HandleConfigCommand(const std::vector<std::string> &tokens) {
    if (tokens.size() == 7 && "get" == Utils::ToLowerCase(tokens[4])) {
        Config &config = Config::getInstance();
        const std::string &config_key = tokens[6];
        const std::string config_value = config.get(config_key);
        return "nil" == config_value ? NIL_RESPONSE : CreateRESP(config_key, config_value);
    }
    return SYNTAX_ERROR;
}

std::string RedisParser::HandleKeysCommand(const std::vector<std::string> &tokens) {
    if (tokens.size() == 5 && "*" == tokens[4]) {
        Config &config = Config::getInstance();
        const std::string path = config.get("dir") + "/" + config.get("dbfilename");
        rdb_reader_.Process(path);
        const auto keys = rdb_reader_.GetKeys();
        return ToArrayString(keys);
    }
    return INVALID_COMMAND_ERROR;
}

std::string RedisParser::HandleTypeCommand(const std::vector<std::string> &tokens) {
    const std::string &key = tokens[4];
    const std::string value = redis_database_.get(key);

    if ("nil" == value) {
        return ToSimpleString("none");
    }
    return ToSimpleString("string");
}


std::string RedisParser::CreateRESP(const std::string &key, const std::string &value) {
    return "*2\r\n$" + std::to_string(key.size()) + "\r\n" + key + "\r\n" +
           "$" + std::to_string(value.size()) + "\r\n" + value + "\r\n";
}

std::string RedisParser::ToBulkString(const std::string &val) {
    return "$" + std::to_string(val.length()) + "\r\n" + val + "\r\n";
}

std::string RedisParser::ToSimpleString(const std::string &val) {
    return "+" + val + "\r\n";
}

std::string RedisParser::ToArrayString(const std::vector<std::string> &tokens) {
    std::string result = "*" + std::to_string(tokens.size()) + "\r\n";

    for (const auto &token: tokens) {
        result += ToBulkString(token);
    }
    return result;
}

std::vector<std::string> RedisParser::ParseTokens(const std::string &input) {
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
