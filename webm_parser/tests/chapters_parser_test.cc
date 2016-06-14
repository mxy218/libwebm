#include "src/chapters_parser.h"

#include "gtest/gtest.h"

#include "test_utils/element_parser_test.h"
#include "webm/id.h"

using webm::ChaptersParser;
using webm::ElementParserTest;
using webm::Id;

namespace {

class ChaptersParserTest
    : public ElementParserTest<ChaptersParser, Id::kChapters> {};

TEST_F(ChaptersParserTest, DefaultValues) {
  ParseAndVerify();

  SetReaderData({
      0x45, 0xB9,  // ID = 0x45B9 (EditionEntry).
      0x80,        // Size = 0.
  });
  ParseAndVerify();
}

TEST_F(ChaptersParserTest, RepeatedValues) {
  SetReaderData({
      0x45, 0xB9,  // ID = 0x45B9 (EditionEntry).
      0x84,        // Size = 4.

      0x45, 0xBC,  //   ID = 0x45BC (EditionUID).
      0x81,        //   Size = 1.
      0x01,        //   Body (value = 1).

      0x45, 0xB9,  // ID = 0x45B9 (EditionEntry).
      0x84,        // Size = 4.

      0x45, 0xBC,  //   ID = 0x45BC (EditionUID).
      0x81,        //   Size = 1.
      0x02,        //   Body (value = 2).
  });

  ParseAndVerify();
}

}  // namespace
