#include "CommandExecutor.h"
#include <algorithm>
#include <iostream>
#include "../database/RedisDatabase.h"
#include "../parser/RespParser.h"
#include "../parser/RespTypes.h"
#include "../utils/Utils.h"
#include "../config/Config.h"

std::unordered_map<std::string, CommandExecutor::CommandHandler> CommandExecutor::command_handler_ = {
    {"PING", HandlePingCommand},
    {"ECHO", HandleEchoCommand},
    {"GET", HandleGetCommand},
    {"SET", HandleSetCommand},
    {"CONFIG", HandleConfigCommand},
    {"KEYS", HandleKeyCommand},
    {"TYPE", HandleTypeCommand}
};

std::string CommandExecutor::Executor(const std::string &input) {
    const RespEntry entry = RespParser::ParseCommand(input);

    if (entry.type != RespType::array) {
        return RespParser::ToError("ERR Protocol error: expected array for command");
    }

    const auto &items = std::get<std::vector<RespEntry> >(entry.value);

    if (items.empty() || items[0].type != RespType::bulk_string) {
        return RespParser::ToError("ERR Protocol error: invalid command array");
    }

    auto cmd = std::get<std::string>(items[0].value);
    std::ranges::transform(cmd, cmd.begin(), ::toupper);

    if (const auto it = command_handler_.find(cmd); it != command_handler_.end()) {
        return it->second(items);
    }
    return RespParser::ToError("ERR wrong Command");
}

std::string CommandExecutor::HandlePingCommand(const std::vector<RespEntry> &entries) {
    return RespParser::ToSimpleString("PONG");
}

std::string CommandExecutor::HandleEchoCommand(const std::vector<RespEntry> &entries) {
    if (entries.size() != 2 || entries[1].type != RespType::bulk_string) {
        return RespParser::ToError("ERR wrong number of arguments for 'echo' command");
    }
    const auto &arg = std::get<std::string>(entries[1].value);
    return RespParser::ToBulkString(arg);
}

std::string CommandExecutor::HandleGetCommand(const std::vector<RespEntry> &entries) {
    if (entries.size() != 2 || entries[1].type != RespType::bulk_string) {
        return RespParser::ToError("ERR wrong number of arguments for 'get' command");
    }

    const auto &key = std::get<std::string>(entries[1].value);

    const auto &value = RedisDatabase::GetInstance().Get(key);

    return "nil" != value ? RespParser::ToBulkString(value) : RespParser::NilResponse();
}

std::string CommandExecutor::HandleSetCommand(const std::vector<RespEntry> &entries) {
    if (entries.size() < 3) {
        return RespParser::ToError("ERR wrong number of arguments for 'set' command");
    }

    const std::string key = std::get<std::string>(entries[1].value);
    const std::string value = std::get<std::string>(entries[2].value);

    if (entries.size() == 3) {
        RedisDatabase::GetInstance().Set(key, value);
        return RespParser::OkResponse();
    }

    if (entries.size() == 5) {
        auto option = std::get<std::string>(entries[3].value);
        std::ranges::transform(option, option.begin(), ::toupper);

        if (option != "PX" && option != "EX") {
            return RespParser::ToError("ERR syntax error");
        }

        const std::string ttl_str = std::get<std::string>(entries[4].value);

        if (!Utils::IsNumeric(ttl_str)) {
            return RespParser::ToError("ERR value is not an integer or out of range");
        }

        const int64_t ttl = option == "PX" ? std::stoll(ttl_str) : std::stoll(ttl_str) * 1000;

        RedisDatabase::GetInstance().Set(key, value, ttl);
        return RespParser::OkResponse();
    }

    return RespParser::ToError("ERR syntax error");
}

std::string CommandExecutor::HandleConfigCommand(const std::vector<RespEntry> &entries) {
    if (entries.size() != 3) {
        return RespParser::ToError("ERR wrong number of arguments for 'config' command");
    }

    auto sub_cmd = std::get<std::string>(entries[1].value);
    std::ranges::transform(sub_cmd, sub_cmd.begin(), ::toupper);

    if (sub_cmd != "GET") {
        return RespParser::ToError("ERR unknown subcommand");
    }

    Config &config = Config::GetInstance();

    const auto key = std::get<std::string>(entries[2].value);
    const std::string value = config.Get(key);

    const std::vector result{key, value};

    return RespParser::ToArrayString(result);
}

std::string CommandExecutor::HandleKeyCommand(const std::vector<RespEntry> &entries) {
    if (entries.size() != 2) {
        return RespParser::ToError("ERR wrong number of arguments for 'keys' command");
    }
    const auto keys = RedisDatabase::GetInstance().GetAllKeys();

    return RespParser::ToArrayString(keys);
}

std::string CommandExecutor::HandleTypeCommand(const std::vector<RespEntry> &entries) {
    if (entries.size() != 2) {
        return RespParser::ToError("ERR wrong number of arguments for 'type' command");
    }

    const auto key = std::get<std::string>(entries[1].value);
    const std::string value = RedisDatabase::GetInstance().Get(key);

    if ("nil" == value) {
        return RespParser::ToSimpleString("none");
    }
    return RespParser::ToSimpleString("string");
}
