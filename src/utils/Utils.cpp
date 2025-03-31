#include "Utils.h"
#include <algorithm>
#include <chrono>
#include <string>


std::string Utils::ToLowerCase(const std::string &input) {
    std::string result = input;
    std::ranges::transform(result, result.begin(), [](const unsigned char c) {
        return std::tolower(c);
    });
    return result;
}

std::optional<size_t> Utils::FindCRLF(const std::string &input) {
    size_t pos = input.find("\r\n");
    return (pos != std::string::npos) ? std::make_optional(pos) : std::nullopt;
}

bool Utils::IsNumeric(const std::string &input) {
    return std::ranges::all_of(input, ::isdigit);
}

int64_t Utils::GetCurrentTimestamp() {
    const auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()
    ).count();
}
