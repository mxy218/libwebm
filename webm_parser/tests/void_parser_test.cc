#include "src/void_parser.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "test_utils/element_parser_test.h"
#include "webm/element.h"
#include "webm/id.h"
#include "webm/status.h"

using testing::NotNull;

using webm::ElementParserTest;
using webm::Id;
using webm::kUnknownElementSize;
using webm::Status;
using webm::VoidParser;

namespace {

class VoidParserTest : public ElementParserTest<VoidParser, Id::kVoid> {};

TEST_F(VoidParserTest, InvalidSize) {
  TestInit(kUnknownElementSize, Status::kInvalidElementSize);
}

TEST_F(VoidParserTest, Empty) {
  EXPECT_CALL(callback_, OnVoid(metadata_, NotNull(), NotNull())).Times(1);

  ParseAndVerify();
}

TEST_F(VoidParserTest, Valid) {
  SetReaderData({0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07});

  EXPECT_CALL(callback_, OnVoid(metadata_, NotNull(), NotNull())).Times(1);

  ParseAndVerify();
}

TEST_F(VoidParserTest, IncrementalParse) {
  SetReaderData({0x00, 0x01, 0x02, 0x03});

  EXPECT_CALL(callback_, OnVoid(metadata_, NotNull(), NotNull())).Times(4);

  IncrementalParseAndVerify();
}

}  // namespace
