#include "src/content_encodings_parser.h"

#include "gtest/gtest.h"

#include "test_utils/element_parser_test.h"
#include "webm/id.h"

using webm::ContentEncoding;
using webm::ContentEncodings;
using webm::ContentEncodingsParser;
using webm::ElementParserTest;
using webm::Id;

namespace {

class ContentEncodingsParserTest
    : public ElementParserTest<ContentEncodingsParser, Id::kContentEncodings> {
};

TEST_F(ContentEncodingsParserTest, DefaultParse) {
  ParseAndVerify();

  const ContentEncodings content_encodings = parser_.value();

  EXPECT_EQ(0, content_encodings.encodings.size());
}

TEST_F(ContentEncodingsParserTest, DefaultValues) {
  SetReaderData({
      0x62, 0x40,  // ID = 0x6240 (ContentEncoding).
      0x80,        // Size = 0.
  });

  ParseAndVerify();

  const ContentEncodings content_encodings = parser_.value();

  ASSERT_EQ(1, content_encodings.encodings.size());
  EXPECT_TRUE(content_encodings.encodings[0].is_present());
  EXPECT_EQ(ContentEncoding{}, content_encodings.encodings[0].value());
}

TEST_F(ContentEncodingsParserTest, CustomValues) {
  SetReaderData({
      0x62, 0x40,  // ID = 0x6240 (ContentEncoding).
      0x84,        // Size = 4.

      0x50, 0x31,  //   ID = 0x5031 (ContentEncodingOrder).
      0x81,        //   Size = 1.
      0x01,        //   Body (value = 1).

      0x62, 0x40,  // ID = 0x6240 (ContentEncoding).
      0x84,        // Size = 4.

      0x50, 0x31,  //   ID = 0x5031 (ContentEncodingOrder).
      0x81,        //   Size = 1.
      0x02,        //   Body (value = 2).
  });

  ParseAndVerify();

  const ContentEncodings content_encodings = parser_.value();

  ContentEncoding expected;

  ASSERT_EQ(2, content_encodings.encodings.size());
  expected.order.Set(1, true);
  EXPECT_TRUE(content_encodings.encodings[0].is_present());
  EXPECT_EQ(expected, content_encodings.encodings[0].value());
  expected.order.Set(2, true);
  EXPECT_TRUE(content_encodings.encodings[1].is_present());
  EXPECT_EQ(expected, content_encodings.encodings[1].value());
}

}  // namespace
