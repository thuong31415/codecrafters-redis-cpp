#include "CommandExecutor.h"
#include <algorithm>
#include <iostream>
#include "../database/RedisDatabase.h"
#include "../parser/RespParser.h"
#include "../parser/RespTypes.h"
#include "../utils/Utils.h"
#include "../config/Config.h"
#include "../database/RedisStream.h"

std::unordered_map<std::string, CommandExecutor::CommandHandler> CommandExecutor::command_handler_ = {
    {"PING", HandlePingCommand},
    {"ECHO", HandleEchoCommand},
    {"GET", HandleGetCommand},
    {"SET", HandleSetCommand},
    {"CONFIG", HandleConfigCommand},
    {"KEYS", HandleKeyCommand},
    {"TYPE", HandleTypeCommand},
    {"XADD", HandleXAddCommand},
    {"XRANGE", HandleXRangeCommand},
    {"XREAD", HandleXReadCommand}
};

std::string CommandExecutor::Executor(const std::string &input) {
    const RespEntry entry = RespParser::ParseCommand(input);

    if (entry.type != RespType::array) {
        return RespParser::Error("ERR Protocol error: expected array for command");
    }

    const auto &items = std::get<std::vector<RespEntry> >(entry.value);

    if (items.empty() || items[0].type != RespType::bulk_string) {
        return RespParser::Error("ERR Protocol error: invalid command array");
    }

    auto cmd = std::get<std::string>(items[0].value);
    std::ranges::transform(cmd, cmd.begin(), ::toupper);

    if (const auto it = command_handler_.find(cmd); it != command_handler_.end()) {
        return it->second(items);
    }
    return RespParser::Error("ERR wrong Command");
}

std::string CommandExecutor::HandlePingCommand(const std::vector<RespEntry> &entries) {
    return RespParser::ToSimpleString("PONG");
}

std::string CommandExecutor::HandleEchoCommand(const std::vector<RespEntry> &entries) {
    if (entries.size() != 2 || entries[1].type != RespType::bulk_string) {
        return RespParser::Error("ERR wrong number of arguments for 'echo' command");
    }
    const auto &arg = std::get<std::string>(entries[1].value);
    return RespParser::ToBulkString(arg);
}

std::string CommandExecutor::HandleGetCommand(const std::vector<RespEntry> &entries) {
    if (entries.size() != 2 || entries[1].type != RespType::bulk_string) {
        return RespParser::Error("ERR wrong number of arguments for 'get' command");
    }

    const auto &key = std::get<std::string>(entries[1].value);

    const auto &value = RedisDatabase::GetInstance().Get(key);

    return "nil" != value ? RespParser::ToBulkString(value) : RespParser::Nil();
}

std::string CommandExecutor::HandleSetCommand(const std::vector<RespEntry> &entries) {
    if (entries.size() < 3) {
        return RespParser::Error("ERR wrong number of arguments for 'set' command");
    }

    const std::string key = std::get<std::string>(entries[1].value);
    const std::string value = std::get<std::string>(entries[2].value);

    if (entries.size() == 3) {
        RedisDatabase::GetInstance().Set(key, value);
        return RespParser::Ok();
    }

    if (entries.size() == 5) {
        auto option = std::get<std::string>(entries[3].value);
        std::ranges::transform(option, option.begin(), ::toupper);

        if (option != "PX" && option != "EX") {
            return RespParser::Error("ERR syntax error");
        }

        const std::string ttl_str = std::get<std::string>(entries[4].value);

        if (!Utils::IsNumeric(ttl_str)) {
            return RespParser::Error("ERR value is not an integer or out of range");
        }

        const int64_t ttl = option == "PX" ? std::stoll(ttl_str) : std::stoll(ttl_str) * 1000;

        RedisDatabase::GetInstance().Set(key, value, ttl);
        return RespParser::Ok();
    }

    return RespParser::Error("ERR syntax error");
}

std::string CommandExecutor::HandleConfigCommand(const std::vector<RespEntry> &entries) {
    if (entries.size() != 3) {
        return RespParser::Error("ERR wrong number of arguments for 'config' command");
    }

    auto sub_cmd = std::get<std::string>(entries[1].value);
    std::ranges::transform(sub_cmd, sub_cmd.begin(), ::toupper);

    if (sub_cmd != "GET") {
        return RespParser::Error("ERR unknown subcommand");
    }

    Config &config = Config::GetInstance();

    const auto key = std::get<std::string>(entries[2].value);
    const std::string value = config.Get(key);

    const std::vector result{key, value};

    return RespParser::ToArrayString(result);
}

std::string CommandExecutor::HandleKeyCommand(const std::vector<RespEntry> &entries) {
    if (entries.size() != 2) {
        return RespParser::Error("ERR wrong number of arguments for 'keys' command");
    }
    const auto keys = RedisDatabase::GetInstance().GetAllKeys();

    return RespParser::ToArrayString(keys);
}

std::string CommandExecutor::HandleTypeCommand(const std::vector<RespEntry> &entries) {
    if (entries.size() != 2) {
        return RespParser::Error("ERR wrong number of arguments for 'type' command");
    }

    const auto key = std::get<std::string>(entries[1].value);
    const std::string value = RedisDatabase::GetInstance().Get(key);

    if (RedisStream::GetInstance().ExistStreamKey(key)) {
        return RespParser::ToSimpleString("stream");
    }

    if ("nil" == value) {
        return RespParser::ToSimpleString("none");
    }
    return RespParser::ToSimpleString("string");
}

std::string CommandExecutor::HandleXAddCommand(const std::vector<RespEntry> &entries) {
    if (entries.size() != 5) {
        return RespParser::Error("ERR wrong number of arguments for 'xadd' command");
    }

    const auto stream_key = std::get<std::string>(entries[1].value);
    const auto entry_id = std::get<std::string>(entries[2].value);
    const auto key = std::get<std::string>(entries[3].value);
    const auto value = std::get<std::string>(entries[4].value);

    return RedisStream::GetInstance().Add(stream_key, entry_id, key, value);
}

std::string CommandExecutor::HandleXRangeCommand(const std::vector<RespEntry> &entries) {
    if (entries.size() != 4) {
        return RespParser::Error("ERR wrong number of arguments for 'xrange' command");
    }

    const auto stream_key = std::get<std::string>(entries[1].value);
    const auto start_key = std::get<std::string>(entries[2].value);
    const auto end_key = std::get<std::string>(entries[3].value);

    const std::vector<StreamEntry> stream_entries = RedisStream::GetInstance().GetByStreamKey(stream_key);

    if (stream_entries.empty()) {
        return RespParser::Empty();
    }

    const StreamEntry entry_start = "-" == start_key ? stream_entries.front() : StreamEntry{start_key};
    const StreamEntry entry_end = "+" == end_key ? stream_entries.back() : StreamEntry{end_key};

    std::vector<StreamEntry> result{};

    for (const auto &entry: stream_entries) {
        if (entry >= entry_start && entry <= entry_end) {
            result.push_back(entry);
        }
    }

    if (result.empty()) {
        return RespParser::Empty();
    }

    std::string resp = "*" + std::to_string(result.size()) + "\r\n";

    for (const auto &item: result) {
        resp += item.ToResp();
    }
    return resp;
}

std::string CommandExecutor::HandleXReadCommand(const std::vector<RespEntry> &entries) {

    if (entries.size() < 4) {
        return RespParser::Error("ERR wrong number of arguments for 'xread' command");
    }

    auto sub_command = std::get<std::string>(entries[1].value);
    std::ranges::transform(sub_command, sub_command.begin(), ::toupper);

    if (sub_command.empty() || sub_command != "STREAMS") {
        return RespParser::Error("ERR syntax error");
    }

    std::vector<std::string> stream_keys{}, entry_ids{};

    for (int i = 2; i < entries.size(); i++) {
        auto data = std::get<std::string>(entries[i].value);
        if (data.find("-") != std::string::npos) {
            entry_ids.push_back(data);
        } else {
            stream_keys.push_back(data);
        }
    }

    if (stream_keys.size() != entry_ids.size()) {
        return RespParser::Error("ERR Unbalanced 'xread' list of streams: for each stream key an ID or '$' must be specified.");
    }

    std::string resp = "*" + std::to_string(stream_keys.size()) + "\r\n";

    for (int i = 0; i < static_cast<int>(stream_keys.size()); i++) {

        const auto stream_key = stream_keys[i];
        const auto entry_id = entry_ids[i];

        const auto stream_entries = RedisStream::GetInstance().GetByStreamKey(stream_key);
        if (stream_entries.empty()) {
            continue;
        }

        const StreamEntry stream_entry{entry_id};
        std::vector<StreamEntry> find_results{};

        for (const auto &entry: stream_entries) {
            if (entry >= stream_entry) {
                find_results.push_back(entry);
            }
        }

        if (find_results.empty()) {
            continue;
        }

        resp += "*2\r\n";
        resp += RespParser::ToBulkString(stream_key);
        resp += "*" + std::to_string(find_results.size()) + "\r\n";

        for (const auto &item: find_results) {
            resp += item.ToResp();
        }
    }

    return resp;
}
