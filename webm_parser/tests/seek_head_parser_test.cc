#include "src/seek_head_parser.h"

#include "gtest/gtest.h"

#include "test_utils/element_parser_test.h"
#include "webm/buffer_reader.h"

using webm::ElementParserTest;
using webm::Id;
using webm::SeekHeadParser;

namespace {

class SeekHeadParserTest
    : public ElementParserTest<SeekHeadParser, Id::kSeekHead> {};

TEST_F(SeekHeadParserTest, DefaultValues) {
  ParseAndVerify();

  SetReaderData({
      0x4D, 0xBB,  // ID = 0x4DBB (Seek).
      0x80,        // Size = 0.
  });
  ParseAndVerify();
}

TEST_F(SeekHeadParserTest, RepeatedValues) {
  SetReaderData({
      0x4D, 0xBB,  // ID = 0x4DBB (Seek).
      0x84,        // Size = 4.

      0x53, 0xAC,  //   ID = 0x53AC (SeekPosition).
      0x81,        //   Size = 1.
      0x01,        //   Body (value = 1).

      0x4D, 0xBB,  // ID = 0x4DBB (Seek).
      0x84,        // Size = 4.

      0x53, 0xAC,  //   ID = 0x53AC (SeekPosition).
      0x81,        //   Size = 1.
      0x02,        //   Body (value = 2).
  });

  ParseAndVerify();
}

}  // namespace
