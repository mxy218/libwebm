// Copyright (c) 2015 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
#ifndef LIBWEBM_WEBM2PES_H_
#define LIBWEBM_WEBM2PES_H_

#include <cstdio>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "mkvparser.hpp"
#include "mkvreader.hpp"

namespace libwebm {

// Stores a value and its size in bits for writing into a PES Optional Header.
// Maximum size is 64 bits. Users may call the Check() method to perform minimal
// validation (size > 0 and <= 64).
struct PesHeaderField {
  PesHeaderField(std::uint64_t value,
                 std::uint32_t size_in_bits,
                 std::uint8_t byte_index,
                 std::uint8_t bits_to_shift)
      : bits(value),
        num_bits(size_in_bits),
        index(byte_index),
        shift(bits_to_shift) {}
  PesHeaderField() = delete;
  PesHeaderField(const PesHeaderField&) = default;
  PesHeaderField(PesHeaderField&&) = default;
  ~PesHeaderField() = default;
  bool Check() const {
    return num_bits > 0 && num_bits <= 64 && shift >= 0 && shift < 64;
  }

  // Value to be stored in the field.
  std::uint64_t bits;

  // Number of bits in the value.
  const std::uint32_t num_bits;

  // Index into the header for the byte in which |bits| will be written.
  const std::uint8_t index;

  // Number of bits to shift value before or'ing.
  const std::uint8_t shift;
};

// Storage for PES Optional Header values. Fields written in order using sizes
// specified.
struct PesOptionalHeader {
  // TODO(tomfinegan): The fields could be in an array, which would allow the
  // code writing the optional header to iterate over the fields instead of
  // having code for dealing with each one.

  // 2 bits (marker): 2 ('10')
  const PesHeaderField marker = PesHeaderField(2, 2, 0, 6);

  // 2 bits (no scrambling): 0x0 ('00')
  const PesHeaderField scrambling = PesHeaderField(0, 2, 0, 4);

  // 1 bit (priority): 0x0 ('0')
  const PesHeaderField priority = PesHeaderField(0, 1, 0, 3);

  // TODO(tomfinegan): The BCMV header could be considered a sync word, and this
  // field should be 1 when a sync word follows the packet. Clarify.
  // 1 bit (data alignment): 0x0 ('0')
  const PesHeaderField data_alignment = PesHeaderField(0, 1, 0, 2);

  // 1 bit (copyright): 0x0 ('0')
  const PesHeaderField copyright = PesHeaderField(0, 1, 0, 1);

  // 1 bit (original/copy): 0x0 ('0')
  const PesHeaderField original = PesHeaderField(0, 1, 0, 0);

  // 1 bit (has_pts): 0x1 ('1')
  const PesHeaderField has_pts = PesHeaderField(1, 1, 1, 7);

  // 1 bit (has_dts): 0x0 ('0')
  const PesHeaderField has_dts = PesHeaderField(0, 1, 1, 6);

  // 6 bits (unused fields): 0x0 ('000000')
  const PesHeaderField unused = PesHeaderField(0, 6, 1, 0);

  // 8 bits (size of remaining data in the Header).
  const PesHeaderField remaining_size = PesHeaderField(6, 8, 2, 0);

  // PTS: 5 bytes
  //   4 bits (flag: PTS present, but no DTS): 0x2 ('0010')
  //   36 bits (90khz PTS):
  //     top 3 bits
  //     marker ('1')
  //     middle 15 bits
  //     marker ('1')
  //     bottom 15 bits
  //     marker ('1')
  PesHeaderField pts = PesHeaderField(0, 40, 3, 0);

  PesHeaderField stuffing_byte = PesHeaderField(0xFF, 8, 8, 0);

  // PTS omitted in fragments. Size remains unchanged: More stuffing bytes.
  bool fragment = false;

  static std::size_t size_in_bytes() { return 9; }

  // Writes |pts_90khz| to |pts| per format described at its declaration above.
  void SetPtsBits(std::int64_t pts_90khz);

  // Writes fields to |file| and returns true. Returns false when write or
  // field value validation fails.
  bool Write(std::FILE* file, bool write_pts) const;
};

// Describes custom 10 byte header that immediately follows the PES Optional
// Header in each PES packet output by Webm2Pes:
//   4 byte 'B' 'C' 'M' 'V'
//   4 byte big-endian length of frame
//   2 bytes 0 padding
struct BCMVHeader {
  explicit BCMVHeader(std::uint32_t frame_length) : length(frame_length) {}
  BCMVHeader() = delete;
  BCMVHeader(const BCMVHeader&) = delete;
  BCMVHeader(BCMVHeader&&) = delete;
  ~BCMVHeader() = default;
  const std::uint8_t bcmv[4] = {'B', 'C', 'M', 'V'};
  const std::uint32_t length;

  static std::size_t size() { return 10; }

  // Write the BCMV Header into the FILE stream.
  bool Write(std::FILE* fileptr) const;
};

struct PesHeader {
  const std::uint8_t start_code[4] = {
      0x00,
      0x00,
      0x01,   // 0x000001 is the PES packet start code prefix.
      0xE0};  // 0xE0 is the minimum video stream ID.
  std::uint16_t packet_length = 0;  // Number of bytes _after_ this field.
  PesOptionalHeader optional_header;
  std::size_t size() const {
    return optional_header.size_in_bytes() + BCMVHeader::size() +
           6 /* start_code + packet_length */ + packet_length;
  }

  // Writes out the header to |file|. Calls PesOptionalHeader::Write() to write
  // |optional_header| contents. Returns true when successful, false otherwise.
  bool Write(std::FILE* file, bool write_pts) const;
};

// Converts the VP9 track of a WebM file to a Packetized Elementary Stream
// suitable for use in a MPEG2TS.
// https://en.wikipedia.org/wiki/Packetized_elementary_stream
// https://en.wikipedia.org/wiki/MPEG_transport_stream
class Webm2Pes {
 public:
  enum VideoCodec { VP8, VP9 };

  Webm2Pes(const std::string& input_file, const std::string& output_file)
      : input_file_name_(input_file), output_file_name_(output_file) {}

  Webm2Pes() = delete;
  Webm2Pes(const Webm2Pes&) = delete;
  Webm2Pes(Webm2Pes&&) = delete;
  ~Webm2Pes() = default;

  // Converts the VPx video stream to a PES and returns true. Returns false
  // to report failure.
  bool Convert();

 private:
  // fclose functor for wrapping FILE in std::unique_ptr.
  struct FILEDeleter {
    int operator()(FILE* f) {
      if (f != nullptr)
        return fclose(f);
      return 0;
    }
  };
  typedef std::unique_ptr<FILE, FILEDeleter> FilePtr;

  bool WritePesPacket(const mkvparser::Block::Frame& vpx_frame,
                      double nanosecond_pts);

  const std::string input_file_name_;
  const std::string output_file_name_;
  std::unique_ptr<mkvparser::Segment> webm_parser_;
  mkvparser::MkvReader webm_reader_;
  FilePtr output_file_;

  // Video track num in the WebM file.
  int video_track_num_ = 0;

  // Video codec reported by CodecName from Video TrackEntry.
  VideoCodec codec_;

  // Input timecode scale.
  std::int64_t timecode_scale_ = 1000000;
};
}  // namespace libwebm

#endif  // LIBWEBM_WEBM2PES_H_
