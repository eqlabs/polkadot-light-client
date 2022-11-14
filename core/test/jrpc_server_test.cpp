#include <gtest/gtest.h>

#include <iostream>
#include <fstream>
#include <thread>

#include "network/json_rpc/jrpc_server.h"

#include "testutils/prepare_loggers.h"

class JrpcServerTest : public testing::Test {
public:
    JrpcServerTest() {
        prepareLoggers();
        system("cd ./nodeutils && npm i");
    }

    void SetUp() override {
    }
    void TearDown() override {
    }

protected:
};

TEST_F(JrpcServerTest, ShouldReturnCorrectResponse) {
    using namespace std::chrono_literals;

    std::shared_ptr<boost::asio::io_service> io = std::make_shared<boost::asio::io_service>();

    auto jrpc_server = std::make_shared<plc::core::network::json_rpc::JrpcServer>(2584, io);
    // TODO
    // rewrite tests when handlers get added to new JrpcServer

    // system("cd ./nodeutils && node json-rpc-client.js '{\"jsonrpc\": \"2.0\",\"id\": 12, \"method\": \"add\", \"params\": [55,34]}' > ./rpc_test.txt");
    // system("cd ./nodeutils && node json-rpc-client.js '{\"jsonrpc\": \"2.0\",\"id\": 12, \"method\": \"multiply\", \"params\": [11,9]}' >> ./rpc_test.txt");
    // system("cd ./nodeutils && node json-rpc-client.js '{\"jsonrpc\": \"2.0\",\"id\": 12, \"method\": \"pow\", \"params\": [64,2]}' >> ./rpc_test.txt");

    // std::ifstream t("./nodeutils/rpc_test.txt");
    // std::string rpc_test((std::istreambuf_iterator<char>(t)),
    //              std::istreambuf_iterator<char>());
    // auto bad = rpc_test.find("\"result\":4095");
    // auto r1 = rpc_test.find("\"result\":89");
    // auto r2 = rpc_test.find("\"result\":99");
    // auto r3 = rpc_test.find("\"result\":4096");
    // EXPECT_EQ(bad, std::string::npos);
    // EXPECT_NE(r1, std::string::npos);
    // EXPECT_NE(r2, std::string::npos);
    // EXPECT_NE(r3, std::string::npos);

    // system("rm -r ./nodeutils/node_modules");
    // system("rm ./nodeutils/package-lock.json");
    // system("rm ./nodeutils/rpc_test.txt");
    // io->stop();
    // io_thread.join();

}
