#include <gtest/gtest.h>

#include "chain/spec.h"
#include "utils/hex.h"

#include "testutils/prepare_loggers.h"

class ChainSpecParseTest : public testing::Test {
public:
    ChainSpecParseTest() {
        prepareLoggers();
        chainSpec = std::make_shared<plc::core::chain::Spec>();
    }

    void SetUp() override {}
    void TearDown() override {}

protected:
    std::shared_ptr<plc::core::chain::Spec> chainSpec;
};

TEST_F(ChainSpecParseTest, ParseSpec) {
    plc::core::Result<void> result = chainSpec->loadFromFile("assets/chain.json");
    EXPECT_FALSE(result.has_error());
    EXPECT_EQ("Polkadot CC1", chainSpec->getName());
    EXPECT_EQ("Live", chainSpec->getChainType());
    EXPECT_EQ("polkadot", chainSpec->getId());
    EXPECT_EQ("dot", chainSpec->getProtocolId());

    auto telemetryEndPoints = chainSpec->getTelemetryEndpoints();
    EXPECT_EQ(1, telemetryEndPoints.size());
    EXPECT_EQ("wss://telemetry.polkadot.io/submit/", telemetryEndPoints[0].first);
    EXPECT_EQ(0, telemetryEndPoints[0].second);

    auto properties = chainSpec->getProperties();
    EXPECT_EQ(3, properties.size());
    EXPECT_TRUE(properties.contains("ss58Format"));
    EXPECT_EQ("0", properties["ss58Format"]);
    EXPECT_TRUE(properties.contains("tokenDecimals"));
    EXPECT_EQ("12", properties["tokenDecimals"]);
    EXPECT_TRUE(properties.contains("tokenSymbol"));
    EXPECT_EQ("DOT", properties["tokenSymbol"]);

    auto genesis = chainSpec->getGenesis();
    EXPECT_EQ(3, genesis.size());

    EXPECT_EQ(1, genesis[0].first.size());
    EXPECT_EQ(1, genesis[0].second.size());
    EXPECT_EQ(0x9c, genesis[0].first[0]);
    EXPECT_EQ(0x00, genesis[0].second[0]);

    EXPECT_EQ(2, genesis[1].first.size());
    EXPECT_EQ(1, genesis[1].second.size());
    EXPECT_EQ(0x9c, genesis[1].first[0]);
    EXPECT_EQ(0x5d, genesis[1].first[1]);
    EXPECT_EQ(0x01, genesis[1].second[0]);

    EXPECT_EQ(2, genesis[2].first.size());
    EXPECT_EQ(2, genesis[2].second.size());
    EXPECT_EQ(0x79, genesis[2].first[0]);
    EXPECT_EQ(0x5a, genesis[2].first[1]);
    EXPECT_EQ(0x0d, genesis[2].second[0]);
    EXPECT_EQ(0x1a, genesis[2].second[1]);

    auto bootNodes = chainSpec->getBootNodes();
    EXPECT_EQ(2, bootNodes.size());
    EXPECT_EQ("/dns4/p2p.cc1-0.polkadot.network/tcp/30100/p2p/12D3KooWEdsXX9657ppNqqrRuaCHFvuNemasgU5msLDwSJ6WqsKc",
              bootNodes[0].getStringAddress());
    EXPECT_EQ("/dns4/cc1-1.parity.tech/tcp/30333/p2p/12D3KooWFN2mhgpkJsDBuNuE5427AcDrsib8EoqGMZmkxWwx3Md4",
              bootNodes[1].getStringAddress());

    auto badBlocks = chainSpec->getBadBlocks();
    EXPECT_EQ(2, badBlocks.size());

    int foundBlocks = 0;

    for (auto block: badBlocks) {
        auto hexValue = plc::core::hex(std::vector(block.begin(), block.end()));
        if (block[0] == 0x15) {
            EXPECT_EQ("0x15b1b925b0aa5cfe43c88cd024f74258cb5cfe3af424882c901014e8acd0d241", hexValue);
            ++foundBlocks;
        }
        if (block[0] == 0x25) {
            EXPECT_EQ("0x2563260209012232649ab9dc003f62e274c684037de499a23062f8e0e816c605", hexValue);
            ++foundBlocks;
        }
    }
    EXPECT_EQ(2, foundBlocks);

}
