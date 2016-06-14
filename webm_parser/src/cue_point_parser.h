#ifndef SRC_CUE_POINT_PARSER_H_
#define SRC_CUE_POINT_PARSER_H_

#include "src/cue_track_positions_parser.h"
#include "src/int_parser.h"
#include "src/master_value_parser.h"
#include "webm/dom_types.h"
#include "webm/id.h"

namespace webm {

// Spec reference:
// http://matroska.org/technical/specs/index.html#CuePoint
// http://www.webmproject.org/docs/container/#CuePoint
class CuePointParser : public MasterValueParser<CuePoint> {
 public:
  CuePointParser()
      : MasterValueParser<CuePoint>(
            MakeChild<UnsignedIntParser>(Id::kCueTime, &CuePoint::time),
            MakeChild<CueTrackPositionsParser>(
                Id::kCueTrackPositions, &CuePoint::cue_track_positions)) {}

 protected:
  Status OnParseCompleted(Callback* callback) override {
    return callback->OnCuePoint(metadata(Id::kCuePoint), value());
  }
};

}  // namespace webm

#endif  // SRC_CUE_POINT_PARSER_H_
