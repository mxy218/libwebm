// Copyright (c) 2015 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
#include "vpxpes2ts.h"

namespace libwebm {
// TODO(tomfinegan): Dedupe this and PesHeaderField.
// Stores a value and its size in bits for writing into a MPEG2 TS Header.
// Maximum size is 64 bits. Users may call the Check() method to perform minimal
// validation (size > 0 and <= 64).
struct TsHeaderField {
  TsHeaderField(std::uint64_t value,
                std::uint32_t size_in_bits,
                std::uint8_t byte_index,
                std::uint8_t bits_to_shift)
      : bits(value),
        num_bits(size_in_bits),
        index(byte_index),
        shift(bits_to_shift) {}
  TsHeaderField() = delete;
  TsHeaderField(const TsHeaderField&) = default;
  TsHeaderField(TsHeaderField&&) = default;
  ~TsHeaderField() = default;
  bool Check() const {
    return num_bits > 0 && num_bits <= 64 && shift >= 0 && shift < 64;
  }

  // Value to be stored in the field.
  std::uint64_t bits;

  // Number of bits in the value.
  const std::uint32_t num_bits;

  // Index into the header for the byte in which |bits| will be written.
  const std::uint8_t index;

  // Number of bits to left shift value before or'ing. Ignored for whole bytes.
  const std::uint8_t shift;
};

// Data storage for MPEG2 Transport Stream headers.
// https://en.wikipedia.org/wiki/MPEG_transport_stream#Packet
struct TsHeader {
  TsHeader(bool payload_start, uint8_t counter)
      : is_payload_start(payload_start), counter_value(counter) {}
  TsHeader() = delete;
  TsHeader(const TsHeader&) = default;
  TsHeader(TsHeader&&) = default;
  ~TsHeader() = default;

  // Indicates the packet is the beginning of a new fragmented payload.
  const bool is_payload_start;

  // The sync byte is the bit pattern of 0x47 (ASCII char 'G').
  const std::uint8_t kTsHeaderSyncByte = 0x47;
  const std::uint8_t sync_byte = kTsHeaderSyncByte;

  // Value for |continuity_counter|. Used to detect gaps when demuxing.
  const std::uint8_t counter_value;

  // Set when FEC is impossible. We don't do Forward Error Correction. Always 0.
  const TsHeaderField transport_error_indicator = TsHeaderField(0, 1, 1, 7);

  // This MPEG2 TS header is the start of a new payload (aka PES packet).
  const TsHeaderField payload_unit_start_indicator =
      TsHeaderField(is_payload_start ? 1 : 0, 1, 1, 6);

  // Set when the current packet has a higher priority than other packets with
  // the same PID. Always 0 for VPX.
  const TsHeaderField transport_priority = TsHeaderField(0, 1, 1, 5);

  // https://en.wikipedia.org/wiki/MPEG_transport_stream#Packet_Identifier_.28PID.29
  // 0x0020-0x1FFA May be assigned as needed to Program Map Tables, elementary
  // streams and other data tables.
  // We hard code to 0x20, so write 0's for the remaining bits of the first byte
  // (|pid_pad|), then write the actual value out (|pid|).
  const TsHeaderField pid_pad = TsHeaderField(0, 5, 1, 4);
  const TsHeaderField pid = TsHeaderField(0x20, 8, 2, 0);

  // Indicates scrambling key. Unused; always 0.
  const TsHeaderField scrambling_control = TsHeaderField(0, 2, 3, 6);

  // Adaptation field flag. Unused; always 0.
  const TsHeaderField adaptation_field_flag = TsHeaderField(0, 1, 3, 5);

  // Payload flag. All output packets created here have payloads. Always 1.
  const TsHeaderField payload_flag = TsHeaderField(1, 1, 3, 4);

  // Continuity counter. Two bit field that is incremented for every packet.
  const TsHeaderField continuity_counter =
      TsHeaderField(counter_value, 2, 3, 3);
};

bool VpxPes2Ts::ConvertToFile() {
  output_file_ = FilePtr(fopen(output_file_name_.c_str(), "wb"), FILEDeleter());
  if (output_file_ == nullptr) {
    std::fprintf(stderr, "VpxPes2Ts: Cannot open %s for output.\n",
                 output_file_name_.c_str());
    return false;
  }
  pes_converter_.reset(new Webm2Pes(input_file_name_, this));
  if (pes_converter_ == nullptr) {
    std::fprintf(stderr, "VpxPes2Ts: Out of memory.\n");
    return false;
  }
  return pes_converter_->ConvertToPacketReceiver();
}

bool VpxPes2Ts::ReceivePacket(const PacketInfo& packet_info,
                              const PacketDataBuffer& packet_data) {
  const std::size_t kTsHeaderSize = 4;
  const std::size_t kTsPayloadSize = 184;
  const std::size_t kTsPacketSize = 188;
  const std::size_t bytes_to_packetize = packet_data.size();

  return false;
}

}  // namespace libwebm
