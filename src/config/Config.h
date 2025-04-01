#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <unordered_map>

class Config {
public:
    Config(const Config &) = delete;

    void operator=(const Config &) = delete;

    static Config &getInstance() {
        static Config instance;
        return instance;
    }

    void set(const std::string &key, const std::string &value);

    std::string get(const std::string &key);

private:
    Config();

    std::unordered_map<std::string, std::string> config_{};
};


#endif //CONFIG_H
