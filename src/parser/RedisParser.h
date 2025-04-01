#ifndef REDIS_PARSER_H
#define REDIS_PARSER_H
#include <string>
#include <vector>

#include "../config/Config.h"
#include "../database/RedisDatabase.h"

class RedisParser {
public:
    static std::string HandleCommand(const std::string &input);
private:
    static RedisDatabase redis_database_;

    static std::vector<std::string> parseTokens(const std::string &input);

    static std::string HandleSimpleString(const std::string &input);

    static std::string HandleError(const std::string &input);

    static std::string HandleInteger(const std::string &input);

    static std::string HandleBulkString(const std::string &input);

    static std::string HandleArrayCommand(const std::string &input);

    static  std::string CreateRESP(const std::string& key, const std::string& value);
};


#endif //REDIS_PARSER_H
