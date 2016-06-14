#include "src/cue_point_parser.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include "test_utils/element_parser_test.h"
#include "webm/id.h"

using webm::CuePoint;
using webm::CuePointParser;
using webm::CueTrackPositions;
using webm::ElementParserTest;
using webm::Id;

namespace {

class CuePointParserTest
    : public ElementParserTest<CuePointParser, Id::kCuePoint> {};

TEST_F(CuePointParserTest, DefaultParse) {
  EXPECT_CALL(callback_, OnCuePoint(metadata_, CuePoint{})).Times(1);

  ParseAndVerify();
}

TEST_F(CuePointParserTest, DefaultValues) {
  SetReaderData({
      0xB3,  // ID = 0xB3 (CueTime).
      0x80,  // Size = 0.

      0xB7,  // ID = 0xB7 (CueTrackPositions).
      0x80,  // Size = 0.
  });

  CuePoint cue_point;
  cue_point.time.Set(0, true);
  cue_point.cue_track_positions.emplace_back();
  cue_point.cue_track_positions[0].Set({}, true);

  EXPECT_CALL(callback_, OnCuePoint(metadata_, cue_point)).Times(1);

  ParseAndVerify();
}

TEST_F(CuePointParserTest, CustomValues) {
  SetReaderData({
      0xB3,  // ID = 0xB3 (CueTime).
      0x81,  // Size = 1.
      0x01,  // Body (value = 1).

      0xB7,  // ID = 0xB7 (CueTrackPositions).
      0x83,  // Size = 3.

      0xF1,  //   ID = 0xF1 (CueClusterPosition).
      0x81,  //   Size = 1.
      0x02,  //   Body (value = 2).

      0xB7,  // ID = 0xB7 (CueTrackPositions).
      0x83,  // Size = 3.

      0xF7,  //   ID = 0xF7 (CueTrack).
      0x81,  //   Size = 1.
      0x03,  //   Body (value = 3).
  });

  CuePoint cue_point;
  cue_point.time.Set(1, true);
  CueTrackPositions cue_track_positions;
  cue_track_positions.cluster_position.Set(2, true);
  cue_point.cue_track_positions.emplace_back(cue_track_positions, true);
  cue_track_positions = {};
  cue_track_positions.track.Set(3, true);
  cue_point.cue_track_positions.emplace_back(cue_track_positions, true);

  EXPECT_CALL(callback_, OnCuePoint(metadata_, cue_point)).Times(1);

  ParseAndVerify();
}

}  // namespace
