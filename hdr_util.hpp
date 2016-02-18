// Copyright (c) 2016 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
#ifndef LIBWEBM_COMMON_HDR_UTIL_H_
#define LIBWEBM_COMMON_HDR_UTIL_H_

#include <memory>

#include "mkvmuxer.hpp"
#include "mkvparser.hpp"

namespace libwebm {

// Utility types and functions for workings with the Colour element and its
// children. Copiers return true upon success. Presence functions return true
// when the specified element is present.

// TODO(tomfinegan): These should be moved to libwebm_utils once c++11 is
// required by libwebm.

typedef std::auto_ptr<mkvmuxer::PrimaryChromaticity> PrimaryChromaticityPtr;

bool CopyPrimaryChromaticity(const mkvparser::PrimaryChromaticity& parser_pc,
                             PrimaryChromaticityPtr* muxer_pc);

bool MasteringMetadataValuePresent(double value);

bool CopyMasteringMetadata(const mkvparser::MasteringMetadata& parser_mm,
                           mkvmuxer::MasteringMetadata* muxer_mm);

bool ColourValuePresent(long long value);

bool CopyColour(const mkvparser::Colour& parser_colour,
                mkvmuxer::Colour* muxer_colour);

}  // namespace libwebm

#endif  // LIBWEBM_COMMON_HDR_UTIL_H_
