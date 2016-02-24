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
#include <memory>
#include <vector>

namespace libwebm {

// fclose functor for wrapping FILE in std::unique_ptr.
struct FILEDeleter {
  int operator()(FILE* f) {
    if (f != nullptr)
      return fclose(f);
    return 0;
  }
};
typedef std::unique_ptr<FILE, FILEDeleter> FilePtr;

struct Range {
  Range(size_t off, size_t len) : offset(off), length(len) {}
  Range() = delete;
  Range(const Range&) = default;
  Range(Range&&) = default;
  ~Range() = default;
  const size_t offset;
  const size_t length;
};

typedef std::vector<Range> Ranges;

// Converts |nanoseconds| to 90000 Hz clock ticks and returns the value.
int64_t NanosecondsTo90KhzTicks(int64_t nanoseconds);

// Returns true and stores frame offsets and lengths in |frame_ranges| when
// |frame| has a valid VP9 super frame index.
bool ParseVP9SuperFrameIndex(const uint8_t* frame,
                             size_t frame_length,
                             Ranges* frame_ranges);

// Writes |val| to |fileptr| and returns true upon success.
bool WriteUint8(uint8_t val, std::FILE* fileptr);

}  // namespace libwebm

#endif  // LIBWEBM_COMMON_LIBWEBM_UTILS_H_
