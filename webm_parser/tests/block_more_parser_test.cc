#include "src/block_more_parser.h"

#include "gtest/gtest.h"

#include "test_utils/element_parser_test.h"
#include "webm/id.h"

using webm::BlockMore;
using webm::BlockMoreParser;
using webm::ElementParserTest;
using webm::Id;

namespace {

class BlockMoreParserTest
    : public ElementParserTest<BlockMoreParser, Id::kBlockMore> {};

TEST_F(BlockMoreParserTest, DefaultParse) {
  ParseAndVerify();

  const BlockMore block_more = parser_.value();

  EXPECT_FALSE(block_more.id.is_present());
  EXPECT_EQ(1, block_more.id.value());

  EXPECT_FALSE(block_more.data.is_present());
  EXPECT_EQ(std::vector<std::uint8_t>{}, block_more.data.value());
}

TEST_F(BlockMoreParserTest, DefaultValues) {
  SetReaderData({
      0xEE,  // ID = 0xEE (BlockAddID).
      0x80,  // Size = 0.

      0xA5,  // ID = 0xA5 (BlockAdditional).
      0x80,  // Size = 0.
  });

  ParseAndVerify();

  const BlockMore block_more = parser_.value();

  EXPECT_TRUE(block_more.id.is_present());
  EXPECT_EQ(1, block_more.id.value());

  EXPECT_TRUE(block_more.data.is_present());
  EXPECT_EQ(std::vector<std::uint8_t>{}, block_more.data.value());
}

TEST_F(BlockMoreParserTest, CustomValues) {
  SetReaderData({
      0xEE,  // ID = 0xEE (BlockAddID).
      0x81,  // Size = 1.
      0x02,  // Body (value = 2).

      0xA5,  // ID = 0xA5 (BlockAdditional).
      0x81,  // Size = 1.
      0x00,  // Body.
  });

  ParseAndVerify();

  const BlockMore block_more = parser_.value();

  EXPECT_TRUE(block_more.id.is_present());
  EXPECT_EQ(2, block_more.id.value());

  EXPECT_TRUE(block_more.data.is_present());
  EXPECT_EQ(std::vector<std::uint8_t>{0x00}, block_more.data.value());
}

}  // namespace
