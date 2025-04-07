#include "servers/Server.h"
#include "config/Config.h"

int main(const int argc, char *argv[]) {

    Config& config = Config::getInstance();

    for (int i = 0; i < argc; i++) {
        if ("--dir" == std::string(argv[i])) {
            config.set("dir", argv[i + 1]);
        }
        if ("--dbfilename" == std::string(argv[i])) {
            config.set("dbfilename", argv[i + 1]);
        }
    }

    Server server{6379};
    server.Start();
    return 0;
}
