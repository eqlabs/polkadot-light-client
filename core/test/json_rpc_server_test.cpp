#include <gtest/gtest.h>

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

#include "network/json_rpc_server.h"

#include "testutils/prepare_loggers.h"

class JsonRpcServerTest : public testing::Test {
public:
    JsonRpcServerTest() {
        prepareLoggers();
        system("cd ./nodeutils && npm i");
    }

    void SetUp() override {
    }
    void TearDown() override {
    }

protected:
};

TEST_F(JsonRpcServerTest, ShouldReturnCorrectResponse) {
    using namespace std::chrono_literals;

    std::shared_ptr<boost::asio::io_service> io = std::make_shared<boost::asio::io_service>();

    auto json_rpc_server = std::make_shared<network::JsonRpcServer>("127.0.0.1", 2584, io);
    auto packio_server = json_rpc_server->getServer();
    packio_server->dispatcher()->add_coro(
        "add", *json_rpc_server->getIoService(), [](int a, int b) -> packio::net::awaitable<int> {
            co_return a + b;
        });
    packio_server->dispatcher()->add_coro(
        "multiply", *json_rpc_server->getIoService(), [](int a, int b) -> packio::net::awaitable<int> {
            co_return a * b;
        });
    packio_server->dispatcher()->add_coro(
        "pow", *json_rpc_server->getIoService(), [](int a, int b) -> packio::net::awaitable<int> {
            co_return std::pow(a, b);
        });

    std::thread io_thread([io](){
        io->run();
    });

    // expect result=89
    system("cd ./nodeutils && node json-rpc-client.js '{\"jsonrpc\": \"2.0\",\"id\": 12, \"method\": \"add\", \"params\": [55,34]}' > ./rpc_test.txt");
    // expect result=99
    system("cd ./nodeutils && node json-rpc-client.js '{\"jsonrpc\": \"2.0\",\"id\": 12, \"method\": \"multiply\", \"params\": [11,9]}' >> ./rpc_test.txt");
    // expect result=4096
    system("cd ./nodeutils && node json-rpc-client.js '{\"jsonrpc\": \"2.0\",\"id\": 12, \"method\": \"pow\", \"params\": [64,2]}' >> ./rpc_test.txt");

    std::ifstream t("./nodeutils/rpc_test.txt");
    std::string rpc_test((std::istreambuf_iterator<char>(t)),
                 std::istreambuf_iterator<char>());
    auto bad = rpc_test.find("\"result\":4095");
    auto r1 = rpc_test.find("\"result\":89");
    auto r2 = rpc_test.find("\"result\":99");
    auto r3 = rpc_test.find("\"result\":4096");
    EXPECT_EQ(bad, std::string::npos);
    EXPECT_NE(r1, std::string::npos);
    EXPECT_NE(r2, std::string::npos);
    EXPECT_NE(r3, std::string::npos);

    system("rm -r ./nodeutils/node_modules");
    system("rm ./nodeutils/package-lock.json");
    system("rm ./nodeutils/rpc_test.txt");
    io->stop();
    io_thread.join();

}
