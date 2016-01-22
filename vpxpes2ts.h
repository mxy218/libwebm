// Copyright (c) 2016 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
#ifndef LIBWEBM_VPXPES2TS_H_
#define LIBWEBM_VPXPES2TS_H_

#include <cstdio>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "common/libwebm_utils.h"
#include "webm2pes.h"

namespace libwebm {

class VpxPes2Ts : public PacketReceiverInterface {
 public:
  VpxPes2Ts(const std::string& input_file_name,
            const std::string& output_file_name)
      : input_file_name_(input_file_name),
        output_file_name_(output_file_name) {}
  virtual ~VpxPes2Ts() = default;
  VpxPes2Ts() = delete;
  VpxPes2Ts(const VpxPes2Ts&) = delete;
  VpxPes2Ts(VpxPes2Ts&&) = delete;

  bool ConvertToFile();

 private:
  virtual bool ReceivePacket(const PacketDataBuffer& packet_data) override;

  const std::string input_file_name_;
  const std::string output_file_name_;

  FilePtr output_file_;
  std::unique_ptr<Webm2Pes> pes_converter_;
  PacketDataBuffer ts_buffer_;
};

}  // namespace libwebm

#endif  // LIBWEBM_VPXPES2TS_H_
