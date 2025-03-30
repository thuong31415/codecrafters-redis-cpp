#include <gtest/gtest.h>
#include "RedisParser.h"

TEST(HandleSimpleString, EmptyInput_ShouldReturnsUnknownCommand) {
    const std::string input{};
    const std::string response = RedisParser::HandleCommand(input);
    EXPECT_EQ("-ERR Unknown command\r\n", response);
}

TEST(HandleSimpleString, ValidOkCommand_ShouldReturnsOk) {
    const std::string input = "+OK\r\n";
    const std::string response = RedisParser::HandleCommand(input);
    EXPECT_EQ("+OK\r\n", response);
}

TEST(HandleSimpleString, ValidPingCommand_ShouldReturnsPong) {
    const std::string input = "+PING\r\n";
    const std::string response = RedisParser::HandleCommand(input);
    EXPECT_EQ("+PONG\r\n", response);
}

TEST(HandleInteger, InValidIntegerCommand_ShouldReturnError) {
    const std::string input = ":hi123\r\n";
    const std::string response = RedisParser::HandleCommand(input);
    EXPECT_EQ("-ERR Invalid integer number\r\n", response);
}

TEST(HandleInteger, ValidIntegerCommand_ShouldReturnValue) {
    const std::string input = ":100\r\n";
    const std::string response = RedisParser::HandleCommand(input);
    EXPECT_EQ(":100\r\n", response);
}

TEST(HandleBulkString, ValidBulkString_houldReturnValue) {
    const std::string input = "$6\r\nfoobar\r\n";
    const std::string response = RedisParser::HandleCommand(input);
    EXPECT_EQ("$6\r\nfoobar\r\n", response);
}

TEST(HandleArray, InputContainSimpleOkCommand_ShouldReturnsOk) {
    const std::string input = "*1\r\n+OK\r\n";
    const std::string response = RedisParser::HandleCommand(input);
    EXPECT_EQ("+OK\r\n", response);
}

TEST(HadnleArray, InputContainPing_ShouldReturnPong) {
    const std::string input = "*1\r\n$4\r\nPING\r\n";
    const std::string response = RedisParser::HandleCommand(input);
    EXPECT_EQ("+PONG\r\n", response);
}

TEST(HandleArray, InputContainEchoCommand_ShouldReturnData) {
    const std::string input = "*2\r\n$4\r\nECHO\r\n$13\r\nHello, Redis!\r\n";
    const std::string response = RedisParser::HandleCommand(input);
    EXPECT_EQ("$13\r\nHello, Redis!\r\n", response);
}

TEST(HandleArray, InPutContainSetCommand_ShouldReturnOk) {
    const std::string input = "*3\r\n$3\r\nSET\r\n$5\r\nmykey\r\n$13\r\nHello, Redis!\r\n";
    const std::string response = RedisParser::HandleCommand(input);
    EXPECT_EQ("+OK\r\n", response);
}

TEST(HandleArray, InputContainGetCommand_ShuoldReturnValue) {
    const std::string command_set_data = "*3\r\n$3\r\nSET\r\n$6\r\nmy_key\r\n$5\r\nhello\r\n";
    RedisParser::HandleCommand(command_set_data);

    const std::string input = "*2\r\n$3\r\nGET\r\n$6\r\nmy_key\r\n";
    const std::string response = RedisParser::HandleCommand(input);
    EXPECT_EQ("$5\r\nhello\r\n", response);
}
