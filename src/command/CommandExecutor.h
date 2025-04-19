#pragma once
#include <string>
#include <functional>
#include "../database/RdbReader.h"
#include "../parser/RespParser.h"

class CommandExecutor {
public:
    CommandExecutor() = default;

    using CommandHandler = std::function<std::string(const std::vector<RespEntry> &entries)>;

    static std::string Executor(const std::string &input);

private:
    RdbReader rdb_reader_{};

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


};
