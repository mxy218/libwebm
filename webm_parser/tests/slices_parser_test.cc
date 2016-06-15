#include "src/slices_parser.h"

#include "gtest/gtest.h"

#include "test_utils/element_parser_test.h"
#include "webm/id.h"

using webm::ElementParserTest;
using webm::Id;
using webm::Slices;
using webm::SlicesParser;
using webm::TimeSlice;

namespace {

class SlicesParserTest : public ElementParserTest<SlicesParser, Id::kSlices> {};

TEST_F(SlicesParserTest, DefaultParse) {
  ParseAndVerify();

  const Slices slices = parser_.value();

  EXPECT_EQ(0, slices.slices.size());
}

TEST_F(SlicesParserTest, DefaultValues) {
  SetReaderData({
      0xE8,  // ID = 0xE8 (TimeSlice).
      0x80,  // Size = 0.
  });

  ParseAndVerify();

  const Slices slices = parser_.value();

  ASSERT_EQ(1, slices.slices.size());
  EXPECT_TRUE(slices.slices[0].is_present());
  EXPECT_EQ(TimeSlice{}, slices.slices[0].value());
}

TEST_F(SlicesParserTest, CustomValues) {
  SetReaderData({
      0xE8,  // ID = 0xE8 (TimeSlice).
      0x83,  // Size = 3.

      0xCC,  //   ID = 0xCC (LaceNumber).
      0x81,  //   Size = 1.
      0x01,  //   Body (value = 1).

      0xE8,  // ID = 0xE8 (TimeSlice).
      0x83,  // Size = 3.

      0xCC,  //   ID = 0xCC (LaceNumber).
      0x81,  //   Size = 1.
      0x02,  //   Body (value = 2).
  });

  ParseAndVerify();

  const Slices slices = parser_.value();

  TimeSlice expected;

  ASSERT_EQ(2, slices.slices.size());
  expected.lace_number.Set(1, true);
  EXPECT_TRUE(slices.slices[0].is_present());
  EXPECT_EQ(expected, slices.slices[0].value());
  expected.lace_number.Set(2, true);
  EXPECT_TRUE(slices.slices[1].is_present());
  EXPECT_EQ(expected, slices.slices[1].value());
}

}  // namespace
