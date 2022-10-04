#include <gtest/gtest.h>

#include "utils/move_on_copy.h"

TEST(CoreLibTest, Move) {
  plc::core::MoveOnCopy<std::unique_ptr<int>> val1(std::make_unique<int>(2));
  auto val2(val1);
  EXPECT_EQ(val1.get(), nullptr);
  ASSERT_NE(val2.get(), nullptr);
  EXPECT_EQ(*val2.get(), 2);
}
