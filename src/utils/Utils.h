#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <optional>

class Utils {
public:
    static std::string ToLowerCase(const std::string &input);

    static std::optional<size_t> FindCRLF(const std::string &input);

    static bool IsNumeric(const std::string &input);
};


#endif //UTILS_H
