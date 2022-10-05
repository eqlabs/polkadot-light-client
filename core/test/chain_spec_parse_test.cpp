#include <gtest/gtest.h>

#include "chain/spec.h"

class ChainSpecParseTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}

protected:
    plc::core::chain::Spec chainSpec;
};

TEST_F(ChainSpecParseTest, ParseSpec) {
    plc::core::Result<void> result = chainSpec.loadFromFile("assets/chain.json");
    EXPECT_FALSE(result.has_error());
    EXPECT_EQ("Polkadot CC1", chainSpec.getName());
    EXPECT_EQ("Live", chainSpec.getChainType());
    EXPECT_EQ("polkadot", chainSpec.getId());
    EXPECT_EQ("dot", chainSpec.getProtocolId());

    auto telemetryEndPoints = chainSpec.getTelemetryEndpoints();
    EXPECT_EQ(1, telemetryEndPoints.size());
    EXPECT_EQ("wss://telemetry.polkadot.io/submit/", telemetryEndPoints[0].first);
    EXPECT_EQ(0, telemetryEndPoints[0].second);

    auto properties = chainSpec.getProperties();
    EXPECT_EQ(3, properties.size());
    EXPECT_TRUE(properties.contains("ss58Format"));
    EXPECT_EQ("0", properties["ss58Format"]);
    EXPECT_TRUE(properties.contains("tokenDecimals"));
    EXPECT_EQ("12", properties["tokenDecimals"]);
    EXPECT_TRUE(properties.contains("tokenSymbol"));
    EXPECT_EQ("DOT", properties["tokenSymbol"]);

    auto genesis = chainSpec.getGenesis();
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

}
