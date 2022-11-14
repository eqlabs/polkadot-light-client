#include <gtest/gtest.h>

#include "chain/spec.h"
#include "runtime/service.h"

#include "mocks/client_runner_mock.h"
#include "mocks/peer_manager_mock.h"
#include "testutils/prepare_loggers.h"

using ::testing::NiceMock;
using ClientRunnerNiceMock = NiceMock<plc::core::ut::mocks::ClientRunnerMock>;
using PeerManagerNiceMock = NiceMock<plc::core::ut::mocks::PeerManagerMock>;

namespace plc::core::ut {

class RuntimeServiceTest : public testing::Test {
public:
    RuntimeServiceTest() {
        prepareLoggers(soralog::Level::DEBUG);
    }

    void SetUp() override {
        m_client_runner_mock = std::make_shared<ClientRunnerNiceMock>();

        auto spec_result = plc::core::chain::Spec::loadFromFile("assets/chain.json");
        EXPECT_FALSE(spec_result.has_error());
        m_chain_spec = std::make_shared<plc::core::chain::Spec>(spec_result.value());

        m_peer_manager_mock = std::make_shared<PeerManagerNiceMock>();

        m_runtime_service = std::make_shared<plc::core::runtime::Service>(m_chain_spec, m_peer_manager_mock, m_client_runner_mock);

    }
    void TearDown() override {}

protected:
    std::shared_ptr<ClientRunnerNiceMock> m_client_runner_mock;
    std::shared_ptr<PeerManagerNiceMock> m_peer_manager_mock;
    std::shared_ptr<plc::core::chain::Spec> m_chain_spec;
    std::shared_ptr<plc::core::runtime::Service> m_runtime_service;

};

TEST_F(RuntimeServiceTest, shouldParseGenesisRuntime) {
    auto result = m_runtime_service->loadGenesisRuntime();
    EXPECT_FALSE(result.has_error());
}

TEST_F(RuntimeServiceTest, shouldReturnCorrectCoreVersion) {
    auto result = m_runtime_service->loadGenesisRuntime();
    EXPECT_FALSE(result.has_error());

    auto runtime_api = m_runtime_service->getRuntimeApi();

    auto coreVersionResult = runtime_api->coreVersion();
    EXPECT_FALSE(coreVersionResult.has_error());

    auto version = coreVersionResult.value();
    EXPECT_EQ("polkadot", version.m_spec_name);
    EXPECT_EQ("parity-polkadot", version.m_impl_name);
}

TEST_F(RuntimeServiceTest, hostApiShouldAllocateMemory) {
    auto result = m_runtime_service->loadGenesisRuntime();
    EXPECT_FALSE(result.has_error());

    auto host_api = m_runtime_service -> getHostApi();

    auto size = wasm::Literal(1);
    auto ptr = host_api->ext_allocator_malloc_version_1({size});
    EXPECT_FALSE(ptr.size() == 0);
    EXPECT_FALSE(ptr[0].geti32() == 0);

    host_api->ext_allocator_free_version_1({ptr[0]});
}

TEST_F(RuntimeServiceTest, hostApiShouldReturnProperLoggingLevel) {
    auto result = m_runtime_service->loadGenesisRuntime();
    EXPECT_FALSE(result.has_error());

    auto host_api = m_runtime_service -> getHostApi();

    auto logging_level = host_api->ext_logging_max_level_version_1({});
    EXPECT_FALSE(logging_level.size() == 0);
    EXPECT_EQ(4 /*debug*/, logging_level[0].geti32());
}

} //namespace plc::core::ut
