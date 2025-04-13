#include "Config.h"


Config::Config() = default;

void Config::Set(const std::string &key, const std::string &value) {
    config_[key] = value;
}

std::string Config::Get(const std::string &key) {
    return !config_.contains(key) ? "nil" : config_[key];
}

std::string Config::GetRdbUrl() {
    return Get("dir") + "/" + Get("dbfilename");
}

