#include "servers/Server.h"

int main() {
    Server server{6379};
    server.Start();
    return 0;
}
