#ifndef REDIS_PARSER_H
#define REDIS_PARSER_H
#include <string>
#include <vector>
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

    static std::string HandleArray(const std::string &input);
};


#endif //REDIS_PARSER_H
