#include <gtest/gtest.h>
#include "RedisParser.h"

TEST(HandleSimpleString, EmptyInput_ShouldReturnsUnknownCommand) {
    RedisParser redis_parser{};
    const std::string input{};
    const std::string response = redis_parser.HandleCommand(input);
    EXPECT_EQ("-ERR Unknown command\r\n", response);
}

TEST(HandleSimpleString, ValidOkCommand_ShouldReturnsOk) {
    RedisParser redis_parser{};
    const std::string input = "+OK\r\n";
    const std::string response = redis_parser.HandleCommand(input);
    EXPECT_EQ("+OK\r\n", response);
}

TEST(HandleSimpleString, ValidPingCommand_ShouldReturnsPong) {
    RedisParser redis_parser{};
    const std::string input = "+PING\r\n";
    const std::string response = redis_parser.HandleCommand(input);
    EXPECT_EQ("+PONG\r\n", response);
}

TEST(HandleInteger, InValidIntegerCommand_ShouldReturnError) {
    RedisParser redis_parser{};
    const std::string input = ":hi123\r\n";
    const std::string response = redis_parser.HandleCommand(input);
    EXPECT_EQ("-ERR Invalid integer number\r\n", response);
}

TEST(HandleInteger, ValidIntegerCommand_ShouldReturnValue) {
    RedisParser redis_parser{};
    const std::string input = ":100\r\n";
    const std::string response = redis_parser.HandleCommand(input);
    EXPECT_EQ(":100\r\n", response);
}

TEST(HandleBulkString, ValidBulkString_houldReturnValue) {
    RedisParser redis_parser{};
    const std::string input = "$6\r\nfoobar\r\n";
    const std::string response = redis_parser.HandleCommand(input);
    EXPECT_EQ("$6\r\nfoobar\r\n", response);
}

TEST(HandleArray, InputContainSimpleOkCommand_ShouldReturnsOk) {
    RedisParser redis_parser{};
    const std::string input = "*1\r\n+OK\r\n";
    const std::string response = redis_parser.HandleCommand(input);
    EXPECT_EQ("+OK\r\n", response);
}

TEST(HadnleArray, InputContainPing_ShouldReturnPong) {
    RedisParser redis_parser{};
    const std::string input = "*1\r\n$4\r\nPING\r\n";
    const std::string response = redis_parser.HandleCommand(input);
    EXPECT_EQ("+PONG\r\n", response);
}

TEST(HandleArray, InputContainEchoCommand_ShouldReturnData) {
    RedisParser redis_parser{};
    const std::string input = "*2\r\n$4\r\nECHO\r\n$13\r\nHello, Redis!\r\n";
    const std::string response = redis_parser.HandleCommand(input);
    EXPECT_EQ("$13\r\nHello, Redis!\r\n", response);
}

TEST(HandleArray, InPutContainSetCommand_ShouldReturnOk) {
    RedisParser redis_parser{};
    const std::string input = "*3\r\n$3\r\nSET\r\n$9\r\nraspberry\r\n$5\r\nmango\r\n";
    const std::string response = redis_parser.HandleCommand(input);
    EXPECT_EQ("+OK\r\n", response);
}

TEST(HandleArray, InputContainGetCommand_ShouldReturnValue) {
    RedisParser redis_parser{};
    const std::string command_set_data =
            "*5\r\n$3\r\nSET\r\n$6\r\nmy_key\r\n$5\r\nhello\r\n$2\r\npx\r\n$3\r\n1000000\r\n";
    redis_parser.HandleCommand(command_set_data);

    const std::string input = "*2\r\n$3\r\nGET\r\n$6\r\nmy_key\r\n";
    const std::string response = redis_parser.HandleCommand(input);
    EXPECT_EQ("$5\r\nhello\r\n", response);
}

TEST(HandleCommandSet, InputSetDataWithTll_ShouldReturnOk) {
    RedisParser redis_parser{};
    const std::string input = "*5\r\n$3\r\nSET\r\n$6\r\norange\r\n$10\r\nstrawberry\r\n$2\r\npx\r\n$3\r\n100\r\n";
    const std::string response = redis_parser.HandleCommand(input);
    EXPECT_EQ("+OK\r\n", response);
}

TEST(HandleCommand, ConfigGetDir_ReturnsCorrectResponse) {
    RedisParser redis_parser{};
    const std::string input = "*3\r\n$6\r\nCONFIG\r\n$3\r\nGET\r\n$3\r\ndir\r\n";
    const std::string response = redis_parser.HandleCommand(input);
    EXPECT_EQ("*2\r\n$3\r\ndir\r\n$16\r\n/tmp/redis-files\r\n", response);
}

TEST(HandleCommand, GetAllKey_ShouldReturnData) {
    RedisParser redis_parser{};
    const std::string input = "*2\r\n$4\r\nKEYS\r\n$1\r\n*\r\n";
    const std::string response = redis_parser.HandleCommand(input);
    EXPECT_EQ("$4\r\nKEYS\r\n$1\r\n", response);
}
