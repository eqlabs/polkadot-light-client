#include <gtest/gtest.h>

#include "utils/hex.h"

TEST(HexTest, Unhex) {
    auto result = plc::core::unhex("1a2b3c");
    EXPECT_FALSE(result.has_error());

    auto vec = result.value();
    EXPECT_EQ(3, vec.size());
    EXPECT_EQ(0x1a, vec[0]);
    EXPECT_EQ(0x2b, vec[1]);
    EXPECT_EQ(0x3c, vec[2]);
}

TEST(HexTest, UnhexShouldFail) {
    auto result = plc::core::unhex("1a2b3");
    EXPECT_TRUE(result.has_error());
    auto error = result.error();
    EXPECT_EQ(static_cast<int>(plc::core::UnhexError::NotEnoughInput), error.value());

    result = plc::core::unhex("1a2b3g");
    EXPECT_TRUE(result.has_error());
    error = result.error();
    EXPECT_EQ(static_cast<int>(plc::core::UnhexError::NonHexInput), error.value());
}

TEST(HexTest, UnhexWith0x) {
    auto result = plc::core::unhexWith0x("0x1a2b3c");
    EXPECT_FALSE(result.has_error());

    auto vec = result.value();
    EXPECT_EQ(3, vec.size());
    EXPECT_EQ(0x1a, vec[0]);
    EXPECT_EQ(0x2b, vec[1]);
    EXPECT_EQ(0x3c, vec[2]);
}

TEST(HexTest, UnhexWith0xShouldFail) {
    auto result = plc::core::unhexWith0x("1a2b3d");
    EXPECT_TRUE(result.has_error());
    auto error = result.error();
    EXPECT_EQ(static_cast<int>(plc::core::UnhexError::Missing0xPrefix), error.value());
}

TEST(HexTest, UnhexWith0xToBlockHash) {
    auto result = plc::core::unhexWith0xToBlockHash("0x1a2b3c");
    EXPECT_FALSE(result.has_error());

    auto vec = result.value();
    EXPECT_EQ(32, vec.size());
    EXPECT_EQ(0x1a, vec[0]);
    EXPECT_EQ(0x2b, vec[1]);
    EXPECT_EQ(0x3c, vec[2]);
}

TEST(HexTest, Hex) {
    std::vector<uint8_t> vec = {0x1a, 0x2b, 0x3c};
    auto result = plc::core::hex(vec);
    EXPECT_EQ("0x1a2b3c", result);
}
