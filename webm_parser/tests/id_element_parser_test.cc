#include "src/id_element_parser.h"

#include "gtest/gtest.h"

#include "test_utils/element_parser_test.h"
#include "webm/element.h"
#include "webm/id.h"
#include "webm/status.h"

using webm::ElementParserTest;
using webm::Id;
using webm::IdElementParser;
using webm::kUnknownElementSize;
using webm::Status;

namespace {

class IdElementParserTest : public ElementParserTest<IdElementParser> {};

TEST_F(IdElementParserTest, InvalidSize) {
  TestInit(0, Status::kInvalidElementSize);
  TestInit(5, Status::kInvalidElementSize);
  TestInit(kUnknownElementSize, Status::kInvalidElementSize);
}

TEST_F(IdElementParserTest, ValidId) {
  SetReaderData({0xEC});
  ParseAndVerify();
  EXPECT_EQ(Id::kVoid, parser_.value());

  SetReaderData({0x1F, 0x43, 0xB6, 0x75});
  ParseAndVerify();
  EXPECT_EQ(Id::kCluster, parser_.value());
}

TEST_F(IdElementParserTest, IncrementalParse) {
  SetReaderData({0x2A, 0xD7, 0xB1});

  IncrementalParseAndVerify();

  EXPECT_EQ(Id::kTimecodeScale, parser_.value());
}

}  // namespace
