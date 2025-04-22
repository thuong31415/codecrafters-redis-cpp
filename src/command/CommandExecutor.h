#pragma once
#include <condition_variable>
#include <string>
#include <functional>
#include <mutex>

#include "../database/RdbReader.h"
#include "../parser/RespParser.h"

class CommandExecutor {
public:
    CommandExecutor() = default;

    using CommandHandler = std::function<std::string(const std::vector<RespEntry> &entries)>;

    static std::string Executor(const std::string &input);

private:
    RdbReader rdb_reader_{};

    inline static std::mutex mutex_;
    inline static std::condition_variable cv_;
    inline static bool is_blocked_{false};
    inline static bool has_data_{false};

    static std::unordered_map<std::string, CommandHandler> command_handler_;

    static std::string HandlePingCommand(const std::vector<RespEntry> &entries);

    static std::string HandleEchoCommand(const std::vector<RespEntry> &entries);

    static std::string HandleGetCommand(const std::vector<RespEntry> &entries);

    static std::string HandleSetCommand(const std::vector<RespEntry> &entries);

    static std::string HandleConfigCommand(const std::vector<RespEntry> &entries);

    static std::string HandleKeyCommand(const std::vector<RespEntry> &entries);

    static std::string HandleTypeCommand(const std::vector<RespEntry> &entries);

    static std::string HandleXAddCommand(const std::vector<RespEntry> &entries);

    static std::string HandleXRangeCommand(const std::vector<RespEntry> &entries);

    static std::string HandleXReadCommand(const std::vector<RespEntry> &entries);
};
