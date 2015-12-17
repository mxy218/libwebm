// Copyright (c) 2015 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
#ifndef LIBWEBM_COMMON_LIBWEBM_UTILS_H_
#define LIBWEBM_COMMON_LIBWEBM_UTILS_H_

#include <cstdio>
#include <vector>

namespace libwebm {

struct Range {
  Range(std::size_t off, std::size_t len) : offset(off), length(len) {}
  Range() = delete;
  Range(const Range&) = default;
  Range(Range&&) = default;
  ~Range() = default;
  const std::size_t offset;
  const std::size_t length;
};

typedef std::vector<Range> Ranges;

// Converts |nanoseconds| to 90000 Hz clock ticks and returns the value.
std::int64_t NanosecondsTo90KhzTicks(std::int64_t nanoseconds);

// Returns true and stores frame offsets and lengths in |frame_ranges| when
// |frame| has a valid VP9 super frame index.
bool ParseVP9SuperFrameIndex(const std::uint8_t* frame,
                             std::size_t frame_length,
                             Ranges* frame_ranges);

// Writes |val| to |fileptr| and returns true upon success.
bool WriteUint8(std::uint8_t val, std::FILE* fileptr);

}  // namespace libwebm

#endif  // LIBWEBM_COMMON_LIBWEBM_UTILS_H_
