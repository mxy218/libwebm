// Copyright (c) 2016 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
#include "webm2pes.h"

#include <cstdint>
#include <cstdio>
#include <limits>
#include <vector>

#include "gtest/gtest.h"

#include "common/libwebm_utils.h"
#include "testing/test_util.h"

namespace {

struct PesOptionalHeader {
  int marker = 0;
  int scrambling = 0;
  int priority = 0;
  int data_alignment = 0;
  int copyright = 0;
  int original = 0;
  int has_pts = 0;
  int has_dts = 0;
  int unused_fields = 0;
  int remaining_size = 0;
  int pts_dts_flag = 0;
  std::uint64_t pts = 0;
  int stuffing_byte = 0;
};

struct BcmvHeader {
  BcmvHeader() = default;
  ~BcmvHeader() = default;
  BcmvHeader(const BcmvHeader&) = delete;
  BcmvHeader(BcmvHeader&&) = delete;

  // Convenience ctor for quick validation of expected values via operator==
  // after parsing input.
  explicit BcmvHeader(std::uint32_t len) : length(len) {
    id[0] = 'B';
    id[1] = 'C';
    id[2] = 'M';
    id[3] = 'V';
  }

  bool operator==(const BcmvHeader& other) const {
    return (other.length == length && other.id[0] == id[0] &&
            other.id[1] == id[1] && other.id[2] == id[2] &&
            other.id[3] == id[3]);
  }

  bool Valid() const {
    return (length > 0 && id[0] == 'B' && id[1] == 'C' && id[2] == 'M' &&
            id[3] == 'V');
  }

  char id[4] = {0};
  std::uint32_t length = 0;
};

struct PesHeader {
  std::uint16_t packet_length = 0;
  PesOptionalHeader opt_header;
  BcmvHeader bcmv_header;
};

class Webm2PesTests : public ::testing::Test {
 public:
  typedef std::vector<std::uint8_t> PesFileData;

  enum ParseState {
    kParsePesHeader,
    kParsePesOptionalHeader,
    kParseBcmvHeader,
  };

  // Constants for validating known values from input data.
  const std::uint8_t kMinVideoStreamId = 0xE0;
  const std::uint8_t kMaxVideoStreamId = 0xEF;
  const std::size_t kPesHeaderSize = 6;
  const std::size_t kPesOptionalHeaderStartOffset = kPesHeaderSize;
  const std::size_t kPesOptionalHeaderSize = 9;
  const int kPesOptionalHeaderMarkerValue = 0x2;

  Webm2PesTests() : read_pos_(0), parse_state_(kParsePesHeader) {}

  bool CreateAndLoadTestInput() {
    libwebm::Webm2Pes converter(input_file_name_, temp_file_name_.name());
    EXPECT_TRUE(converter.ConvertToFile());
    pes_file_size_ = libwebm::test::GetFileSize(pes_file_name());
    EXPECT_GT(pes_file_size_, 0);
    pes_file_data_.reserve(pes_file_size_);
    EXPECT_EQ(pes_file_size_, pes_file_data_.capacity());
    libwebm::FilePtr file = libwebm::FilePtr(
        std::fopen(pes_file_name().c_str(), "rb"), libwebm::FILEDeleter());
    EXPECT_EQ(std::fread(&pes_file_data_[0], 1, pes_file_size_, file.get()),
              pes_file_size_);
    read_pos_ = 0;
    parse_state_ = kParsePesHeader;
    return true;
  }

  bool VerifyPacketStartCode() {
    // PES packets all start with the byte sequence 0x0 0x0 0x1.
    if (pes_file_data_[read_pos_] != 0 || pes_file_data_[read_pos_ + 1] != 0 ||
        pes_file_data_[read_pos_ + 2] != 1) {
      return false;
    }
    return true;
  }

  std::uint8_t ReadStreamId() { return pes_file_data_[read_pos_ + 3]; }

  std::uint16_t ReadPacketLength() {
    // Read and byte swap 16 bit big endian length.
    return ((pes_file_data_[read_pos_ + 4] & 0xff) << 8) |
           pes_file_data_[read_pos_ + 5];
  }

  void ParsePesHeader(PesHeader* header) {
    EXPECT_TRUE(header != nullptr);
    EXPECT_TRUE(VerifyPacketStartCode());
    // PES Video stream IDs start at E0.
    EXPECT_GE(ReadStreamId(), kMinVideoStreamId);
    EXPECT_LE(ReadStreamId(), kMaxVideoStreamId);
    header->packet_length = ReadPacketLength();
    read_pos_ += kPesHeaderSize;
    parse_state_ = kParsePesOptionalHeader;
  }

  void ParsePesOptionalHeader(PesOptionalHeader* header) {
    EXPECT_TRUE(header != nullptr);
    EXPECT_EQ(kParsePesOptionalHeader, parse_state_);
    std::size_t offset = read_pos_ + kPesOptionalHeaderStartOffset;
    EXPECT_LT(offset, pes_file_size_);

    // Parse the first byte. Spec:
    // https://en.wikipedia.org/wiki/Packetized_elementary_stream
    // TODO(tomfinegan): Make these masks constants.
    header->marker = (pes_file_data_[offset] & 0x80) >> 6;
    EXPECT_EQ(kPesOptionalHeaderMarkerValue, header->marker);
    header->scrambling = (pes_file_data_[offset] & 0x30) >> 4;
    EXPECT_EQ(0, header->scrambling);
    header->priority = (pes_file_data_[offset] & 0x8) >> 3;
    EXPECT_EQ(0, header->priority);
    header->data_alignment = (pes_file_data_[offset] & 0xc) >> 2;
    EXPECT_EQ(0, header->data_alignment);
    header->copyright = (pes_file_data_[offset] & 0x2) >> 1;
    EXPECT_EQ(0, header->copyright);
    header->original = pes_file_data_[offset] & 0x1;
    EXPECT_EQ(0, header->original);

    offset++;
    header->has_pts = (pes_file_data_[offset] & 0x80) >> 7;
    EXPECT_EQ(1, header->has_pts);
    header->has_dts = (pes_file_data_[offset] & 0x40) >> 6;
    EXPECT_EQ(0, header->has_dts);
    header->unused_fields = pes_file_data_[offset] & 0x3f;
    EXPECT_EQ(0, header->unused_fields);

    offset++;
    header->remaining_size = pes_file_data_[offset];
    // Webm2pes always writes 6 bytes
    EXPECT_EQ(6, header->remaining_size);

    int bytes_left = header->remaining_size;

    if (header->has_pts) {
      // Read PTS
      // PTS: 5 bytes
      //   4 bits (flag: PTS present, but no DTS): 0x2 ('0010')
      //   36 bits (90khz PTS):
      //     top 3 bits
      //     marker ('1')
      //     middle 15 bits
      //     marker ('1')
      //     bottom 15 bits
      //     marker ('1')
      // TODO(tomfinegan): Extract some golden timestamp values and actually
      // read the timestamp.
      offset++;
      header->pts_dts_flag = (pes_file_data_[offset] & 0x20) >> 4;
      EXPECT_EQ(0x2, header->pts_dts_flag);
      // Check the marker bits.
      int marker_bit = pes_file_data_[offset] & 1;
      EXPECT_EQ(1, marker_bit);
      offset += 2;
      marker_bit = pes_file_data_[offset] & 1;
      EXPECT_EQ(1, marker_bit);
      offset += 2;
      marker_bit = pes_file_data_[offset] & 1;
      EXPECT_EQ(1, marker_bit);
      bytes_left -= 5;
      offset++;
    }

    // Validate stuffing bytes
    int i;
    for (i = 0; i < bytes_left; ++i)
      EXPECT_EQ(0xff, pes_file_data_[offset + i]);

    offset += i;
    EXPECT_EQ(offset, kPesOptionalHeaderSize);

    read_pos_ += kPesOptionalHeaderSize;
    parse_state_ = kParseBcmvHeader;
  }

  void ParseBcmvHeader(BcmvHeader* header) {
    EXPECT_TRUE(header != nullptr);
    EXPECT_EQ(kParseBcmvHeader, kParseBcmvHeader);
    std::size_t offset = read_pos_;
    header->id[0] = pes_file_data_[offset];
    offset++;
    header->id[1] = pes_file_data_[offset];
    offset++;
    header->id[2] = pes_file_data_[offset];
    offset++;
    header->id[3] = pes_file_data_[offset];
    offset++;

    // TODO(tomfinegan): Read/byteswap BCMV length.

    // TODO(tomfinegan): Verify data instead of jumping to the next packet.
    read_pos_ += header->length;
    parse_state_ = kParsePesHeader;
  }

  ~Webm2PesTests() = default;

  const std::string& pes_file_name() const { return temp_file_name_.name(); }
  std::uint64_t pes_file_size() const { return pes_file_size_; }
  const PesFileData& pes_file_data() const { return pes_file_data_; }

 private:
  const libwebm::test::TempFileDeleter temp_file_name_;
  const std::string input_file_name_ =
      libwebm::test::GetTestFilePath("bbb_480p_vp9_opus_1second.webm");
  std::uint64_t pes_file_size_ = 0;
  PesFileData pes_file_data_;
  std::size_t read_pos_;
  ParseState parse_state_;
};

TEST_F(Webm2PesTests, CreatePesFile) {
  EXPECT_TRUE(CreateAndLoadTestInput());
}

TEST_F(Webm2PesTests, CanParseFirstPacket) {
  EXPECT_TRUE(CreateAndLoadTestInput());

  //
  // Parse the PES Header.
  // This is number of bytes following the final byte in the 5 byte static PES
  // header. PES optional header is 9 bytes. Payload is 83 bytes.
  PesHeader header;
  ParsePesHeader(&header);
  const std::size_t kPesOptionalHeaderLength = 9;
  const std::size_t kFirstFrameLength = 83;
  const std::size_t kPesPayloadLength =
      kPesOptionalHeaderLength + kFirstFrameLength;
  //
  // Parse the PES optional header.
  //
  PesOptionalHeader poh;
  ParsePesOptionalHeader(&poh);
  //  poh.marker =

  //
  // Parse the BCMV Header
  //

  //
  // Update read pos and make sure we see a start code w/sane stream ID
  // following the packet payload (confirming that this packet is fully parsed).
  //
}

}  // namespace

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
