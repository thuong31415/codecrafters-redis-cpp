#include "RedisParser.h"
#include "../utils/Utils.h"

std::vector<std::string> RedisParser::parseTokens(const std::string &input) {
    std::string token = input;
    std::vector<std::string> tokens;

    while (!token.empty()) {
        const size_t pos = token.find("\r\n");
        if (pos == std::string::npos) {
            tokens.push_back(token);
            break;
        }
        tokens.push_back(token.substr(0, pos));
        token.erase(0, pos + 2);
    }
    return tokens;
}

std::string RedisParser::HandleCommand(const std::string &input) {
    const std::vector<std::string> tokens = parseTokens(input);
    for (size_t i = 0; i < tokens.size(); ++i) {
        char firstCharacter = tokens[i][0];

        if (Utils::ToLowerCase(tokens[i]) == "ping") {
            return "+PONG\r\n";
        }
        if (firstCharacter == '+' && Utils::ToLowerCase(tokens[i].substr(1)) == "ping") {
            return "+PONG\r\n";
        }
        if (Utils::ToLowerCase(tokens[i]) == "echo" && i + 2 < tokens.size()) {
            return tokens[i + 1] + "\r\n" + tokens[i + 2] + "\r\n";
        }
    }
    return "-ERR Unknown command\r\n";
}
