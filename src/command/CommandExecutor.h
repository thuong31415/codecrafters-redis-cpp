#pragma once
#include <string>
#include "../database/RdbReader.h"
#include "../database/RedisDatabase.h"
#include "../parser/RespParser.h"

class CommandExecutor {
public:
    CommandExecutor() = default;

    static std::string Executor(const std::string &input);

private:
    RdbReader rdb_reader_{};

    static std::string HandleCommandSimpleString(const RespEntry &entry);

    std::string HandleCommandArray(const RespEntry &entry);

    static std::string HandleEchoCommand(const std::vector<RespEntry> &entries);

    static std::string HandleGetCommand(const std::vector<RespEntry> &entries);

    static std::string HandleSetCommand(const std::vector<RespEntry> &entries);

    static std::string HandleDelCommand(const std::vector<RespEntry> &entries);

    static std::string HandleConfigCommand(const std::vector<RespEntry> &entries);

    static std::string HandleKeyCommand(const std::vector<RespEntry> &entries);

    static std::string HandleTypeCommand(const std::vector<RespEntry> &entries);
};
