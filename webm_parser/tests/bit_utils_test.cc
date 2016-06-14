#include "src/bit_utils.h"

#include "gtest/gtest.h"

using webm::CountLeadingZeros;

namespace {

class BitUtilsTest : public testing::Test {};

TEST_F(BitUtilsTest, CountLeadingZeros) {
  EXPECT_EQ(8, CountLeadingZeros(0x00));
  EXPECT_EQ(4, CountLeadingZeros(0x0f));
  EXPECT_EQ(0, CountLeadingZeros(0xf0));
}

}  // namespace
