#include <gtest/gtest.h>

#include "utils/move_on_copy.h"

TEST(CoreLibTest, Move) {
  plc::core::MoveOnCopy<int> zero;
  plc::core::MoveOnCopy<int> another(std::move(zero));
  EXPECT_EQ(another.get(), 0);
}
