#include "webm/buffer_reader.h"

#include <array>
#include <cstdint>

#include "gtest/gtest.h"

using webm::BufferReader;
using webm::Status;

namespace {

class BufferReaderTest : public testing::Test {};

TEST_F(BufferReaderTest, Assignment) {
  // Test the reader to make sure it resets correctly when assigned.
  std::array<std::uint8_t, 4> buffer;
  std::uint64_t count;
  Status status;

  BufferReader reader({});
  EXPECT_EQ(0, reader.size());

  status = reader.Read(buffer.size(), buffer.data(), &count);
  EXPECT_EQ(Status::kEndOfFile, status.code);
  EXPECT_EQ(0, count);

  reader = {1, 2, 3, 4};
  EXPECT_EQ(4, reader.size());

  status = reader.Read(2, buffer.data(), &count);
  EXPECT_EQ(Status::kOkCompleted, status.code);
  EXPECT_EQ(2, count);

  reader = {5, 6, 7, 8};
  status = reader.Read(2, buffer.data() + 2, &count);
  EXPECT_EQ(Status::kOkCompleted, status.code);
  EXPECT_EQ(2, count);

  std::array<std::uint8_t, 4> expected = {{1, 2, 5, 6}};
  EXPECT_EQ(expected, buffer);
}

TEST_F(BufferReaderTest, Empty) {
  // Test the reader to make sure it reports EOF on empty inputs.
  std::array<std::uint8_t, 1> buffer;
  std::uint64_t count;
  Status status;

  BufferReader reader({});

  status = reader.Read(buffer.size(), buffer.data(), &count);
  EXPECT_EQ(Status::kEndOfFile, status.code);
  EXPECT_EQ(0, count);

  status = reader.Skip(1, &count);
  EXPECT_EQ(Status::kEndOfFile, status.code);
  EXPECT_EQ(0, count);
}

TEST_F(BufferReaderTest, Read) {
  // Test the Read method to make sure it reads data correctly.
  std::array<std::uint8_t, 15> buffer{};
  std::uint64_t count;
  Status status;

  BufferReader reader({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});

  status = reader.Read(5, buffer.data(), &count);
  EXPECT_EQ(Status::kOkCompleted, status.code);
  EXPECT_EQ(5, count);

  status = reader.Read(10, buffer.data() + 5, &count);
  EXPECT_EQ(Status::kOkPartial, status.code);
  EXPECT_EQ(5, count);

  std::array<std::uint8_t, 15> expected = {{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}};
  EXPECT_EQ(expected, buffer);

  status = reader.Read(buffer.size(), buffer.data(), &count);
  EXPECT_EQ(Status::kEndOfFile, status.code);
  EXPECT_EQ(0, count);
}

TEST_F(BufferReaderTest, Skip) {
  // Test the Skip method to make sure it skips data correctly.
  std::uint64_t count;
  Status status;

  BufferReader reader({0, 1, 2, 3, 4, 5, 6, 7, 8, 9});

  status = reader.Skip(3, &count);
  EXPECT_EQ(Status::kOkCompleted, status.code);
  EXPECT_EQ(3, count);

  status = reader.Skip(10, &count);
  EXPECT_EQ(Status::kOkPartial, status.code);
  EXPECT_EQ(7, count);

  status = reader.Skip(1, &count);
  EXPECT_EQ(Status::kEndOfFile, status.code);
  EXPECT_EQ(0, count);
}

TEST_F(BufferReaderTest, ReadAndSkip) {
  // Test the Read and Skip methods together to make sure they interact
  // correclty.
  std::array<std::uint8_t, 10> buffer = {};
  std::uint64_t count;
  Status status;

  BufferReader reader({9, 8, 7, 6, 5, 4, 3, 2, 1, 0});

  status = reader.Read(5, buffer.data(), &count);
  EXPECT_EQ(Status::kOkCompleted, status.code);
  EXPECT_EQ(5, count);

  status = reader.Skip(3, &count);
  EXPECT_EQ(Status::kOkCompleted, status.code);
  EXPECT_EQ(3, count);

  status = reader.Read(5, buffer.data() + 5, &count);
  EXPECT_EQ(Status::kOkPartial, status.code);
  EXPECT_EQ(2, count);

  std::array<std::uint8_t, 10> expected = {{9, 8, 7, 6, 5, 1, 0, 0, 0, 0}};
  EXPECT_EQ(expected, buffer);
}

TEST_F(BufferReaderTest, Position) {
  std::array<std::uint8_t, 10> buffer = {};
  std::uint64_t count;
  Status status;

  BufferReader reader({9, 8, 7, 6, 5, 4, 3, 2, 1, 0});
  EXPECT_EQ(0, reader.Position());

  status = reader.Read(5, buffer.data(), &count);
  EXPECT_EQ(Status::kOkCompleted, status.code);
  EXPECT_EQ(5, count);
  EXPECT_EQ(5, reader.Position());

  status = reader.Skip(3, &count);
  EXPECT_EQ(Status::kOkCompleted, status.code);
  EXPECT_EQ(3, count);
  EXPECT_EQ(8, reader.Position());

  status = reader.Read(5, buffer.data() + 5, &count);
  EXPECT_EQ(Status::kOkPartial, status.code);
  EXPECT_EQ(2, count);
  EXPECT_EQ(10, reader.Position());

  std::array<std::uint8_t, 10> expected = {{9, 8, 7, 6, 5, 1, 0, 0, 0, 0}};
  EXPECT_EQ(expected, buffer);
}

}  // namespace
