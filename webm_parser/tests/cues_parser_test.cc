#include "src/cues_parser.h"

#include "gtest/gtest.h"

#include "test_utils/element_parser_test.h"
#include "webm/id.h"

using webm::CuesParser;
using webm::ElementParserTest;
using webm::Id;

namespace {

class CuesParserTest : public ElementParserTest<CuesParser, Id::kCues> {};

TEST_F(CuesParserTest, DefaultValues) {
  ParseAndVerify();

  SetReaderData({
      0xBB,  // ID = 0xBB (CuePoint).
      0x80,  // Size = 0.
  });
  ParseAndVerify();
}

TEST_F(CuesParserTest, RepeatedValues) {
  SetReaderData({
      0xBB,  // ID = 0xBB (CuePoint).
      0x83,  // Size = 3.

      0xB3,  //   ID = 0xB3 (CueTime).
      0x81,  //   Size = 1.
      0x01,  //   Body (value = 1).

      0xBB,  // ID = 0xBB (CuePoint).
      0x83,  // Size = 3.

      0xB3,  //   ID = 0xB3 (CueTime).
      0x81,  //   Size = 1.
      0x02,  //   Body (value = 2).
  });

  ParseAndVerify();
}

}  // namespace
