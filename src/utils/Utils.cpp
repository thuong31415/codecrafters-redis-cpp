#include "Utils.h"
#include <algorithm>
#include <string>


std::string Utils::ToLowerCase(const std::string &input) {
    std::string result = input;
    std::ranges::transform(result, result.begin(), [](const unsigned char c) {
        return std::tolower(c);
    });
    return result;
}

