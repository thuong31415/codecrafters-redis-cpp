#include <iostream>
#include "servers/Server.h"
#include "config/Config.h"
#include "utils/Utils.h"

void ProcessCommandLineArgs(const int argc, char *argv[], Config &config) {
    for (int i = 0; i < argc; i++) {
        if ("--dir" == std::string(argv[i]) && i + 1 < argc) {
            config.Set("dir", argv[i + 1]);
        }
        if ("--dbfilename" == std::string(argv[i]) && i + 1 < argc) {
            config.Set("dbfilename", argv[i + 1]);
        }
    }
}

void LoadRdbData(const std::string &rdb_path) {
    RdbReader rdb_reader;
    rdb_reader.Process(rdb_path);

    const auto &data = rdb_reader.GetData();
    const int64_t current_time = Utils::GetCurrentTimestamp();

    int loaded_keys = 0;

    for (const auto &[key, value_pair]: data) {
        if (const auto &[value, ttl] = value_pair; ttl <= 0 || ttl > current_time) {
            RedisDatabase::GetInstance().Set(key, value, ttl);
            loaded_keys++;
        }
    }
    std::cout << "Loaded " << loaded_keys << " keys from RDB file." << std::endl;
}

int main(const int argc, char *argv[]) {
    Config &config = Config::GetInstance();
    ProcessCommandLineArgs(argc, argv, config);

    try {
        const std::string rdb_path = config.GetRdbUrl();
        LoadRdbData(rdb_path);
    } catch (const std::exception &e) {
        std::cerr << "Error loading RDB file: " << e.what() << std::endl;
    }

    Server server{6379};
    server.Start();
    return 0;
}
