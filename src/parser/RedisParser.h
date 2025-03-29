#ifndef REDIS_PARSER_H
#define REDIS_PARSER_H
#include <string>
#include <vector>

class RedisParser {
public:
    static std::string HandleCommand(const std::string &input);

private:
    static std::vector<std::string> parseTokens(const std::string &input);
};


#endif //REDIS_PARSER_H
