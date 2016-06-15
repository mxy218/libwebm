#include "src/time_slice_parser.h"

#include "gtest/gtest.h"

#include "test_utils/element_parser_test.h"
#include "webm/id.h"

using webm::ElementParserTest;
using webm::Id;
using webm::TimeSlice;
using webm::TimeSliceParser;

namespace {

class TimeSliceParserTest
    : public ElementParserTest<TimeSliceParser, Id::kTimeSlice> {};

TEST_F(TimeSliceParserTest, DefaultParse) {
  ParseAndVerify();

  const TimeSlice time_slice = parser_.value();

  EXPECT_FALSE(time_slice.lace_number.is_present());
  EXPECT_EQ(0, time_slice.lace_number.value());
}

TEST_F(TimeSliceParserTest, DefaultValues) {
  SetReaderData({
      0xCC,  // ID = 0xCC (LaceNumber).
      0x80,  // Size = 0.
  });

  ParseAndVerify();

  const TimeSlice time_slice = parser_.value();

  EXPECT_TRUE(time_slice.lace_number.is_present());
  EXPECT_EQ(0, time_slice.lace_number.value());
}

TEST_F(TimeSliceParserTest, CustomValues) {
  SetReaderData({
      0xCC,  // ID = 0xCC (LaceNumber).
      0x81,  // Size = 1.
      0x01,  // Body (value = 1).
  });

  ParseAndVerify();

  const TimeSlice time_slice = parser_.value();

  EXPECT_TRUE(time_slice.lace_number.is_present());
  EXPECT_EQ(1, time_slice.lace_number.value());
}

}  // namespace
