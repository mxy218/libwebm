#include "src/date_parser.h"

#include "gtest/gtest.h"

#include "test_utils/element_parser_test.h"
#include "webm/status.h"

using webm::DateParser;
using webm::ElementParserTest;
using webm::kUnknownElementSize;
using webm::Status;

namespace {

class DateParserTest : public ElementParserTest<DateParser> {};

TEST_F(DateParserTest, InvalidSize) {
  TestInit(4, Status::kInvalidElementSize);
  TestInit(9, Status::kInvalidElementSize);
  TestInit(kUnknownElementSize, Status::kInvalidElementSize);
}

TEST_F(DateParserTest, CustomDefault) {
  ResetParser(-1);

  ParseAndVerify();

  EXPECT_EQ(-1, parser_.value());
}

TEST_F(DateParserTest, ValidDate) {
  ParseAndVerify();
  EXPECT_EQ(0, parser_.value());

  SetReaderData({0x12, 0x34, 0x56, 0x78, 0x9A, 0xBC, 0xDE, 0xF0});
  ParseAndVerify();
  EXPECT_EQ(0x123456789ABCDEF0, parser_.value());
}

TEST_F(DateParserTest, IncrementalParse) {
  SetReaderData({0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10});

  IncrementalParseAndVerify();

  EXPECT_EQ(0xFEDCBA9876543210, parser_.value());
}

}  // namespace
