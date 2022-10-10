#include <gtest/gtest.h>

#include "chain/spec.h"
#include "utils/hex.h"

#include "testutils/prepare_loggers.h"

class ChainSpecParseTest : public testing::Test {
public:
    ChainSpecParseTest() {
        prepareLoggers();
    }

    void SetUp() override {}
    void TearDown() override {}

protected:
};

TEST_F(ChainSpecParseTest, ShouldParseSpec) {
    auto spec_result = plc::core::chain::Spec::loadFromFile("assets/chain.json");
    EXPECT_FALSE(spec_result.has_error());

    auto chain_spec = spec_result.value();
    EXPECT_EQ("Polkadot CC1", chain_spec.getName());
    EXPECT_EQ("Live", chain_spec.getChainType());
    EXPECT_EQ("polkadot", chain_spec.getId());
    EXPECT_EQ("dot", chain_spec.getProtocolId());

    auto telemetry_end_points = chain_spec.getTelemetryEndpoints();
    EXPECT_EQ(1, telemetry_end_points.size());
    EXPECT_EQ("wss://telemetry.polkadot.io/submit/", telemetry_end_points[0].first);
    EXPECT_EQ(0, telemetry_end_points[0].second);

    auto properties = chain_spec.getProperties();
    EXPECT_EQ(3, properties.size());
    EXPECT_TRUE(properties.contains("ss58Format"));
    EXPECT_EQ("0", properties["ss58Format"]);
    EXPECT_TRUE(properties.contains("tokenDecimals"));
    EXPECT_EQ("12", properties["tokenDecimals"]);
    EXPECT_TRUE(properties.contains("tokenSymbol"));
    EXPECT_EQ("DOT", properties["tokenSymbol"]);

    auto genesis = chain_spec.getGenesis();
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

    auto boot_nodes = chain_spec.getBootNodes();
    EXPECT_EQ(2, boot_nodes.size());
    EXPECT_EQ("/dns4/p2p.cc1-0.polkadot.network/tcp/30100/p2p/12D3KooWEdsXX9657ppNqqrRuaCHFvuNemasgU5msLDwSJ6WqsKc",
              boot_nodes[0].getStringAddress());
    EXPECT_EQ("/dns4/cc1-1.parity.tech/tcp/30333/p2p/12D3KooWFN2mhgpkJsDBuNuE5427AcDrsib8EoqGMZmkxWwx3Md4",
              boot_nodes[1].getStringAddress());

    auto bad_blocks = chain_spec.getBadBlocks();
    EXPECT_EQ(2, bad_blocks.size());

    int found_blocks = 0;

    for (auto block: bad_blocks) {
        auto hex_value = plc::core::hex(std::vector(block.begin(), block.end()));
        if (block[0] == 0x15) {
            EXPECT_EQ("0x15b1b925b0aa5cfe43c88cd024f74258cb5cfe3af424882c901014e8acd0d241", hex_value);
            ++found_blocks;
        }
        if (block[0] == 0x25) {
            EXPECT_EQ("0x2563260209012232649ab9dc003f62e274c684037de499a23062f8e0e816c605", hex_value);
            ++found_blocks;
        }
    }
    EXPECT_EQ(2, found_blocks);
}

TEST_F(ChainSpecParseTest, ShouldFailOnParsingBadSpec) {
    auto spec_result = plc::core::chain::Spec::loadFromFile("assets/bad_chain.json");
    EXPECT_TRUE(spec_result.has_error());

    auto error = spec_result.error();
    EXPECT_EQ(static_cast<int>(plc::core::chain::Spec::Error::ParserError), error.value());
}

TEST_F(ChainSpecParseTest, ShouldFailOnParsingSpecWithMissingId) {
    auto spec_result = plc::core::chain::Spec::loadFromFile("assets/chain_missing_id.json");
    EXPECT_TRUE(spec_result.has_error());

    auto error = spec_result.error();
    EXPECT_EQ(static_cast<int>(plc::core::chain::Spec::Error::MissingEntry), error.value());
}
