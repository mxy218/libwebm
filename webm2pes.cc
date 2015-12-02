// Copyright (c) 2015 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>

#include "mkvparser.hpp"
#include "mkvreader.hpp"

namespace {
struct Range {
  Range(size_t off, size_t len) : offset(off), length(len) {}
  Range() = delete;
  Range(const Range&) = default;
  Range(Range&&) = default;
  ~Range() = default;
  const size_t offset;
  const size_t length;
};
typedef std::vector<Range> FrameRanges;

void Usage(const char* argv[]) {
  printf("Usage: %s <WebM file> <output file>", argv[0]);
}

bool WriteUint8(std::uint8_t val, std::FILE* fileptr) {
  if (fileptr == nullptr) return false;
  return (std::fputc(val, fileptr) == val);
}

// Returns true and stores frame offsets and lengths in |frame_ranges| when
// |frame| has a valid VP9 super frame index.
bool ParseVP9SuperFrameIndex(const std::uint8_t* frame, std::size_t length,
                             FrameRanges* frame_ranges) {
  if (frame == nullptr || length == 0 || frame_ranges == nullptr) return false;

  bool parse_ok = false;
  const std::uint8_t marker = frame[length - 1];
  const std::uint32_t kHasSuperFrameIndexMask = 0xe0;
  const std::uint32_t kSuperFrameMarker = 0xc0;

  if ((marker & kHasSuperFrameIndexMask) == kSuperFrameMarker) {
    const std::uint32_t kFrameCountMask = 0x7;
    const std::uint32_t kFrameLengthFieldSizeMask = 0x18;

    const int num_frames = (marker & kFrameCountMask) + 1;
    const int length_field_size = (marker & kFrameLengthFieldSizeMask) + 1;
    const std::size_t index_length = 2 + length_field_size * num_frames;
    std::size_t frame_offset = 0;

    if (length >= index_length && frame[length - index_length] == marker) {
      // Found a valid superframe index.
      const std::uint8_t* byte = frame + length - index_length + 1;

      for (int i = 0; i < num_frames; ++i) {
        std::uint32_t child_frame_length = 0;

        for (int j = 0; j < length_field_size; ++j) {
          child_frame_length |= (*byte++) << (j * 8);
        }

        frame_ranges->push_back(Range(frame_offset, child_frame_length));
        frame_offset += child_frame_length;
      }

      if (frame_ranges->size() != num_frames) {
        std::fprintf(stderr, "Webm2Pes: superframe index parse failed.\n");
        return false;
      }

      parse_ok = true;
    } else {
      std::fprintf(stderr, "Webm2Pes: Invalid superframe index.\n");
    }
  }
  return parse_ok;
}
}  // namespace

namespace libwebm {

// Stores a value and its size in bits for writing into a PES Optional Header.
// Maximum size is 64 bits. Users may call the Check() method to perform minimal
// validation (size > 0 and <= 64).
struct PesHeaderField {
  PesHeaderField(std::uint64_t value, std::uint32_t size_in_bits,
                 std::uint8_t byte_index, std::uint8_t bits_to_shift)
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
};

struct BCMVHeader {
  explicit BCMVHeader(std::uint32_t frame_length) : length(frame_length) {}
  BCMVHeader() = delete;
  BCMVHeader(const BCMVHeader&) = delete;
  BCMVHeader(BCMVHeader&&) = delete;
  ~BCMVHeader() = default;
  // 4 byte 'B' 'C' 'M' 'V'
  // 4 byte big-endian length of frame
  // 2 bytes 0 padding
  const std::uint8_t bcmv[4] = {'B', 'C', 'M', 'V'};
  const std::uint32_t length;

  static std::size_t size() { return 10; }

  // Write the BCMV Header into the FILE stream.
  bool Write(std::FILE* fileptr) {
    if (fileptr == nullptr) {
      std::fprintf(stderr, "Webm2Pes: nullptr for file in BCMV Write.\n");
      return false;
    }
    if (std::fwrite(bcmv, 1, 4, fileptr) != 4) {
      std::fprintf(stderr, "Webm2Pes: BCMV write failed.\n");
    }
    const std::size_t kRemainingBytes = 6;
    const uint8_t buffer[kRemainingBytes] = {
        (length >> 24) & 0xff, (length >> 16) & 0xff, (length >> 8) & 0xff,
        length & 0xff, 0, 0 /* 2 bytes 0 padding */};
    for (std::size_t i = 0; i < kRemainingBytes; ++i) {
      if (WriteUint8(buffer[i], fileptr) != true) {
        std::fprintf(stderr, "Webm2Pes: BCMV remainder write failed.\n");
        return false;
      }
    }
    return true;
  }
};

struct PesHeader {
  const std::uint8_t start_code[4] = {
      0x00, 0x00,
      0x01,   // 0x000001 is the PES packet start code prefix.
      0xE0};  // 0xE0 is the minimum video stream ID.
  std::uint16_t packet_length = 0;  // Number of bytes _after_ this field.
  PesOptionalHeader optional_header;
  std::size_t size() const {
    return optional_header.size_in_bytes() + BCMVHeader::size() +
           6 /* start_code + packet_length */ + packet_length;
  }
};

// Converts the VP9 track of a WebM file to a Packetized Elementary Stream
// suitable for use in a MPEG2TS.
// https://en.wikipedia.org/wiki/Packetized_elementary_stream
// https://en.wikipedia.org/wiki/MPEG_transport_stream
class Webm2Pes {
 public:
  enum VideoCodec {
    VP8,
    VP9,
  };
  Webm2Pes(const std::string& input_file, const std::string& output_file);
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
      if (f != nullptr) return fclose(f);
      return 0;
    }
  };
  typedef std::unique_ptr<FILE, FILEDeleter> FilePtr;

  bool Write90KHzPtsBitsToPesHeader(std::int64_t pts_90khz,
                                    PesHeader* header) const;
  bool WritePesOptionalHeader(const PesHeader& pes_header, std::size_t length,
                              std::uint8_t* header) const;
  bool WritePesPacket(const mkvparser::Block::Frame& vpx_frame,
                      double timecode_1000nanosecond_ticks);

  const std::string input_file_name_;
  const std::string output_file_name_;
  std::unique_ptr<mkvparser::Segment> webm_parser_;
  mkvparser::MkvReader webm_reader_;
  FilePtr output_file_;

  // Video track num in the WebM file.
  int video_track_num_ = 0;

  // Video codec.
  VideoCodec codec_;

  // Input timecode scale.
  std::int64_t timecode_scale_ = 1000000;
};

Webm2Pes::Webm2Pes(const std::string& input_file_name,
                   const std::string& output_file_name)
    : input_file_name_(input_file_name), output_file_name_(output_file_name) {}

bool Webm2Pes::Convert() {
  if (input_file_name_.empty() || output_file_name_.empty()) {
    std::fprintf(stderr, "Webm2Pes: input and/or output file name(s) empty.\n");
    return false;
  }

  if (webm_reader_.Open(input_file_name_.c_str()) != 0) {
    std::fprintf(stderr, "Webm2Pes: Cannot open %s as input.\n",
                 input_file_name_.c_str());
    return false;
  }

  output_file_ = FilePtr(fopen(output_file_name_.c_str(), "wb"), FILEDeleter());
  if (output_file_ == nullptr) {
    std::fprintf(stderr, "Webm2Pes: Cannot open %s for output.\n",
                 output_file_name_.c_str());
    return false;
  }

  using mkvparser::Segment;
  Segment* webm_parser = nullptr;
  if (Segment::CreateInstance(&webm_reader_, 0 /* pos */,
                              webm_parser /* Segment*& */) != 0) {
    std::fprintf(stderr, "Webm2Pes: Cannot create WebM parser.\n");
    return false;
  }
  webm_parser_.reset(webm_parser);

  if (webm_parser_->Load() != 0) {
    std::fprintf(stderr, "Webm2Pes: Cannot parse %s.\n",
                 input_file_name_.c_str());
    return false;
  }

  // Store timecode scale.
  timecode_scale_ = webm_parser_->GetInfo()->GetTimeCodeScale();

  // Make sure there's a video track.
  const mkvparser::Tracks* tracks = webm_parser_->GetTracks();
  if (tracks == nullptr) {
    std::fprintf(stderr, "Webm2Pes: %s has no tracks.\n",
                 input_file_name_.c_str());
    return false;
  }
  for (int track_index = 0; track_index < tracks->GetTracksCount();
       ++track_index) {
    const mkvparser::Track* track = tracks->GetTrackByIndex(track_index);
    if (track && track->GetType() == mkvparser::Track::kVideo) {
      if (std::string(track->GetCodecNameAsUTF8()) == std::string("V_VP8"))
        codec_ = VP8;
      else if (std::string(track->GetCodecNameAsUTF8()) == std::string("V_VP9"))
        codec_ = VP9;
      else {
        fprintf(stderr, "Webm2Pes: Codec must be VP8 or VP9.\n");
        return false;
      }
      video_track_num_ = track_index + 1;
      break;
    }
  }
  if (video_track_num_ < 1) {
    std::fprintf(stderr, "Webm2Pes: No video track found in %s.\n",
                 input_file_name_.c_str());
    return false;
  }

  // Walk clusters in segment.
  const mkvparser::Cluster* cluster = webm_parser_->GetFirst();
  while (cluster != nullptr && cluster->EOS() == false) {
    const mkvparser::BlockEntry* block_entry = nullptr;
    std::int64_t block_status = cluster->GetFirst(block_entry);
    if (block_status < 0) {
      std::fprintf(stderr, "Webm2Pes: Cannot parse first block in %s.\n",
                   input_file_name_.c_str());
      return false;
    }

    // Walk blocks in cluster.
    while (block_entry != nullptr && block_entry->EOS() == false) {
      const mkvparser::Block* block = block_entry->GetBlock();
      if (block->GetTrackNumber() == video_track_num_) {
        const int frame_count = block->GetFrameCount();

        // Walk frames in block.
        for (int frame_num = 0; frame_num < frame_count; ++frame_num) {
          const mkvparser::Block::Frame& frame = block->GetFrame(frame_num);

          // Write frame out as PES packet(s).
          const bool pes_status =
              WritePesPacket(frame, block->GetTimeCode(cluster));
          if (pes_status != true) {
            std::fprintf(stderr, "Webm2Pes: WritePesPacket failed.\n");
            return false;
          }
        }
      }
      block_status = cluster->GetNext(block_entry, block_entry);
      if (block_status < 0) {
        std::fprintf(stderr, "Webm2Pes: Cannot parse block in %s.\n",
                     input_file_name_.c_str());
        return false;
      }
    }

    cluster = webm_parser_->GetNext(cluster);
  }

  return true;
}

bool Webm2Pes::Write90KHzPtsBitsToPesHeader(std::int64_t pts_90khz,
                                            PesHeader* header) const {
  if (header == nullptr) {
    std::fprintf(stderr, "Webm2Pes: cannot write PTS to nullptr.\n");
    return false;
  }
  PesHeaderField* pts_field = &header->optional_header.pts;
  std::uint64_t* pts_bits = &pts_field->bits;
  *pts_bits = 0;

  // PTS is broken up:
  // bits 32-30
  // marker
  // bits 29-15
  // marker
  // bits 14-0
  // marker
  const std::uint32_t pts1 = (pts_90khz >> 30) & 0x7;
  const std::uint32_t pts2 = (pts_90khz >> 15) & 0x7FFF;
  const std::uint32_t pts3 = pts_90khz & 0x7FFF;

  std::uint8_t buffer[5] = {0};
  // PTS only flag.
  buffer[0] |= 1 << 5;
  // Top 3 bits of PTS and 1 bit marker.
  buffer[0] |= pts1 << 1;
  // Marker.
  buffer[0] |= 1;

  // Next 15 bits of pts and 1 bit marker.
  // Top 8 bits of second PTS chunk.
  buffer[1] |= (pts2 >> 8) & 0xff;
  // bottom 7 bits of second PTS chunk.
  buffer[2] |= (pts2 << 1);
  // Marker.
  buffer[2] |= 1;

  // Last 15 bits of pts and 1 bit marker.
  // Top 8 bits of second PTS chunk.
  buffer[3] |= (pts3 >> 8) & 0xff;
  // bottom 7 bits of second PTS chunk.
  buffer[4] |= (pts3 << 1);
  // Marker.
  buffer[4] |= 1;

  // Write bits into PesHeaderField.
  std::memcpy(reinterpret_cast<std::uint8_t*>(pts_bits), buffer, 5);
  return true;
}

bool Webm2Pes::WritePesOptionalHeader(const PesHeader& pes_header,
                                      std::size_t length,
                                      std::uint8_t* header) const {
  if (header == nullptr) {
    std::fprintf(stderr, "Webm2Pes: nullptr in opt header writer.\n");
    return false;
  }

  std::uint8_t* byte = header;
  const PesOptionalHeader& poh = pes_header.optional_header;

  if (poh.marker.Check() != true || poh.scrambling.Check() != true ||
      poh.priority.Check() != true || poh.data_alignment.Check() != true ||
      poh.copyright.Check() != true || poh.original.Check() != true ||
      poh.has_pts.Check() != true || poh.has_dts.Check() != true ||
      poh.pts.Check() != true || poh.stuffing_byte.Check() != true) {
    std::fprintf(stderr, "Webm2Pes: Invalid PES Optional Header field.\n");
    return false;
  }

  // TODO(tomfinegan): As noted in PesOptionalHeader, the PesHeaderFields
  // should be an array that can be iterated over.

  // First byte of header, fields: marker, scrambling, priority, alignment,
  // copyright, original.
  *byte = 0;
  *byte |= poh.marker.bits << poh.marker.shift;
  *byte |= poh.scrambling.bits << poh.scrambling.shift;
  *byte |= poh.priority.bits << poh.priority.shift;
  *byte |= poh.data_alignment.bits << poh.data_alignment.shift;
  *byte |= poh.copyright.bits << poh.copyright.shift;
  *byte |= poh.original.bits << poh.original.shift;

  // Second byte of header, fields: has_pts, has_dts, unused fields.
  *++byte = 0;
  *byte |= poh.has_pts.bits << poh.has_pts.shift;
  *byte |= poh.has_dts.bits << poh.has_dts.shift;

  // Third byte of header, fields: remaining size of header.
  *++byte = poh.remaining_size.bits;  // Field is 8 bits wide.

  // Set the PTS value.
  *++byte = (poh.pts.bits >> 32) & 0xff;
  *++byte = (poh.pts.bits >> 24) & 0xff;
  *++byte = (poh.pts.bits >> 16) & 0xff;
  *++byte = (poh.pts.bits >> 8) & 0xff;
  *++byte = poh.pts.bits & 0xff;

  // Add the stuffing byte;
  *++byte = poh.stuffing_byte.bits;

  if (std::fwrite(reinterpret_cast<void*>(header), 1, poh.size_in_bytes(),
                  output_file_.get()) != poh.size_in_bytes()) {
    std::fprintf(stderr, "Webm2Pes: unable to write PES opt header to file.\n");
    return false;
  }

  return true;
}

bool Webm2Pes::WritePesPacket(const mkvparser::Block::Frame& vpx_frame,
                              double timecode_1000nanosecond_ticks) {
  bool packetize = false;
  PesHeader header;
  FrameRanges frame_ranges;
  frame_ranges.push_back(Range(0, vpx_frame.len));

  // Read the input frame.
  std::unique_ptr<uint8_t[]> frame_data(new (std::nothrow)
                                            uint8_t[vpx_frame.len]);
  if (frame_data.get() == nullptr) {
    std::fprintf(stderr, "Webm2Pes: Out of memory.\n");
    return false;
  }
  if (vpx_frame.Read(&webm_reader_, frame_data.get()) != 0) {
    std::fprintf(stderr, "Webm2Pes: Error reading VPx frame!\n");
    return false;
  }

  if (codec_ == VP9) {
    frame_ranges.clear();
    bool has_superframe_index =
        ParseVP9SuperFrameIndex(frame_data.get(), vpx_frame.len, &frame_ranges);
    if (has_superframe_index == false) {
      frame_ranges.push_back(Range(0, vpx_frame.len));
    }
  }

  // TODO(tomfinegan): The length field in PES is actually number of bytes that
  // follow the length field, and does not include the 6 byte fixed portion of
  // the header (4 byte start code + 2 bytes for the length). We can fit in 6
  // more bytes if we really want to, and avoid packetization when size is very
  // close to UINT16_MAX.
  FrameRanges packet_ranges;
  int num_packets = 1;

  for (const Range& range : frame_ranges) {
    if (range.length + header.size() > UINT16_MAX) {

    }
  }

  if (header.size() + vpx_frame.len > UINT16_MAX) {
    packetize = true;
    const int max_packet_size = UINT16_MAX;
    const size_t packet_payload_size = UINT16_MAX - header.size();
    num_packets = vpx_frame.len +
                  (max_packet_size - 1) / (max_packet_size - header.size());
    for (int i = 0; i < num_packets; ++i) {
      packet_ranges.push_back(
          Range((i + 1) * packet_payload_size, packet_payload_size));
    }

    size_t remaining_frame_payload = vpx_frame.len % packet_payload_size;
    if (remaining_frame_payload > 0) {
      packet_ranges.push_back(
          Range(num_packets * packet_payload_size, remaining_frame_payload));
    }
  }

  // TODO(tomfinegan): Need to inspect frame, check for super frame, split it
  // up when one is found w/multiple frames. Need BCMV/PTS for complete and
  // first of a packetized sequence of frames.

  const double kNanosecondsPerSecond = 1000000000.0;
  const double kMillisecondsPerSecond = 1000.0;
  const double k1000NanosecondTicksPerSecond =
      kNanosecondsPerSecond * kMillisecondsPerSecond;
  const double pts_seconds =
      timecode_1000nanosecond_ticks / k1000NanosecondTicksPerSecond;
  const std::int64_t khz90_pts = pts_seconds * 90000;

  if (Write90KHzPtsBitsToPesHeader(khz90_pts, &header) != true) {
    std::fprintf(stderr, "Webm2Pes: 90khz pts write failed.");
    return false;
  }

  std::uint8_t write_buffer[9] = {0};
  if (packetize == false) {
    header.packet_length =
        header.optional_header.size_in_bytes() + vpx_frame.len;
    if (std::fwrite(reinterpret_cast<const void*>(header.start_code), 1, 4,
                    output_file_.get()) != 4) {
      std::fprintf(stderr, "Webm2Pes: cannot write packet start code.\n");
      return false;
    }

    // Big endian length.
    std::uint8_t byte = (header.packet_length >> 8) & 0xff;
    if (WriteUint8(byte, output_file_.get()) != true) {
      std::fprintf(stderr, "Webm2Pes: cannot write packet length (1).\n");
      return false;
    }
    byte = header.packet_length & 0xff;
    if (WriteUint8(byte, output_file_.get()) != true) {
      std::fprintf(stderr, "Webm2Pes: cannot write packet length (2).\n");
      return false;
    }

    if (WritePesOptionalHeader(header, header.packet_length,
                               &write_buffer[0]) != true) {
      std::fprintf(stderr, "Webm2Pes: PES optional header write failed.");
      return false;
    }

    // Write the BCMV Header.
    BCMVHeader bcmv_header(vpx_frame.len);
    if (bcmv_header.Write(output_file_.get()) != true) {
      std::fprintf(stderr, "Webm2Pes: BCMV write failed.\n");
      return false;
    }

    // Write frame.

    if (std::fwrite(frame_data.get(), 1, vpx_frame.len, output_file_.get()) !=
        vpx_frame.len) {
      std::fprintf(stderr, "Webm2Pes: VPx frame write failed.\n");
      return false;
    }

  } else {
    std::fprintf(
        stderr,
        "Webm2Pes: Packetization of large frames not implemented yet.\n");
    return false;
  }

  return true;
}

}  // namespace libwebm

int main(int argc, const char* argv[]) {
  if (argc < 3) {
    Usage(argv);
    return EXIT_FAILURE;
  }

  const std::string input_path = argv[1];
  const std::string output_path = argv[2];

  libwebm::Webm2Pes converter(input_path, output_path);
  return converter.Convert() == true ? EXIT_SUCCESS : EXIT_FAILURE;
}
