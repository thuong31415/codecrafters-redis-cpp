#ifndef REDIS_PARSER_H
#define REDIS_PARSER_H
#include <string>
#include <vector>

#include "../database/RedisDatabase.h"
#include "../database/RdbReader.h"

class RedisParser {
public:
    RedisParser() = default;
    std::string HandleCommand(const std::string &input);
    static std::string ToArrayString(const std::vector<std::string> &tokens);
private:
    RedisDatabase redis_database_{};
    RdbReader rdb_reader_{};

    // RESP Protocol Handlers
    static std::string HandleSimpleString(const std::string &input);
    static std::string HandleError(const std::string &input);
    static std::string HandleInteger(const std::string &input);
    static std::string HandleBulkString(const std::string &input);
    std::string HandleArrayCommand(const std::string &input);

    // Command Handlers
    static std::string HandlePingCommand(const std::vector<std::string>& tokens);
    static std::string HandleEchoCommand(const std::vector<std::string>& tokens);
    std::string HandleSetCommand(const std::vector<std::string>& tokens);
    std::string HandleGetCommand(const std::vector<std::string>& tokens);
    static  std::string HandleConfigCommand(const std::vector<std::string>& tokens);
    std::string HandleKeysCommand(const std::vector<std::string>& tokens);

    // Utility methods
    static std::vector<std::string> ParseTokens(const std::string &input);
    static std::string CreateRESP(const std::string& key, const std::string& value);
    static std::string ToBulkString(const std::string &val);

};


#endif //REDIS_PARSER_H
