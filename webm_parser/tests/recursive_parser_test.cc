#include "src/recursive_parser.h"

#include <cstdint>

#include "gtest/gtest.h"

#include "src/byte_parser.h"
#include "src/element_parser.h"
#include "test_utils/element_parser_test.h"
#include "webm/element.h"
#include "webm/status.h"

using webm::Callback;
using webm::ElementMetadata;
using webm::ElementParser;
using webm::ElementParserTest;
using webm::Reader;
using webm::RecursiveParser;
using webm::Status;
using webm::StringParser;

namespace {

class FailParser : public ElementParser {
 public:
  FailParser() { EXPECT_FALSE(true); }

  Status Init(const ElementMetadata& metadata,
              std::uint64_t max_size) override {
    return Status(Status::kInvalidElementSize);
  }

  Status Feed(Callback* callback, Reader* reader,
              std::uint64_t* num_bytes_read) override {
    *num_bytes_read = 0;
    return Status(Status::kInvalidElementSize);
  }

  int value() const { return 0; }

  int* mutable_value() { return nullptr; }
};

class RecursiveFailParserTest
    : public ElementParserTest<RecursiveParser<FailParser>> {};

TEST_F(RecursiveFailParserTest, NoConstruction) {
  RecursiveParser<FailParser> parser;
}

class RecursiveStringParserTest
    : public ElementParserTest<RecursiveParser<StringParser>> {};

TEST_F(RecursiveStringParserTest, ParsesOkay) {
  ParseAndVerify();
  EXPECT_EQ("", parser_.value());

  SetReaderData({0x48, 0x69});  // "Hi".
  ParseAndVerify();
  EXPECT_EQ("Hi", parser_.value());
}

}  // namespace
