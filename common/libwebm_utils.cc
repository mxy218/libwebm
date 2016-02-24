// Copyright (c) 2015 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
#include "common/libwebm_utils.h"

#include <cstdint>
#include <cstdio>

namespace libwebm {

int64_t NanosecondsTo90KhzTicks(int64_t nanoseconds) {
  const double kNanosecondsPerSecond = 1000000000.0;
  const double pts_seconds = nanoseconds / kNanosecondsPerSecond;
  return pts_seconds * 90000;
}

bool ParseVP9SuperFrameIndex(const uint8_t* frame,
                             size_t frame_length,
                             Ranges* frame_ranges) {
  if (frame == nullptr || frame_length == 0 || frame_ranges == nullptr)
    return false;

  bool parse_ok = false;
  const uint8_t marker = frame[frame_length - 1];
  const uint32_t kHasSuperFrameIndexMask = 0xe0;
  const uint32_t kSuperFrameMarker = 0xc0;
  const uint32_t kLengthFieldSizeMask = 0x3;

  if ((marker & kHasSuperFrameIndexMask) == kSuperFrameMarker) {
    const uint32_t kFrameCountMask = 0x7;
    const int num_frames = (marker & kFrameCountMask) + 1;
    const int length_field_size = ((marker >> 3) & kLengthFieldSizeMask) + 1;
    const size_t index_length = 2 + length_field_size * num_frames;

    if (frame_length < index_length) {
      std::fprintf(stderr, "Webm2Pes: Invalid superframe index size.\n");
      return false;
    }

    // Consume the super frame index. Note: it's at the end of the super frame.
    const size_t length = frame_length - index_length;

    if (length >= index_length &&
        frame[frame_length - index_length] == marker) {
      // Found a valid superframe index.
      const uint8_t* byte = frame + length + 1;

      size_t frame_offset = 0;
      for (int i = 0; i < num_frames; ++i) {
        uint32_t child_frame_length = 0;

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

bool WriteUint8(uint8_t val, std::FILE* fileptr) {
  if (fileptr == nullptr)
    return false;
  return (std::fputc(val, fileptr) == val);
}

}  // namespace libwebm
