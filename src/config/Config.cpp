#include "Config.h"


Config::Config() = default;

void Config::set(const std::string &key, const std::string &value) {
    config_[key] = value;
}

std::string Config::get(const std::string &key) {
    return !config_.contains(key) ? "nil" : config_[key];
}
