// Copyright (c) 2015 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.
#include "vpxpes2ts.h"

namespace libwebm {

bool VpxPes2Ts::ConvertToFile() {
  return false;
}

bool VpxPes2Ts::ReceivePacket(PacketType packet_type,
                              const PacketDataBuffer& packet_data) {
  return false;
}

}  // namespace libwebm
