#ifndef SRC_TRACKS_PARSER_H_
#define SRC_TRACKS_PARSER_H_

#include "src/master_parser.h"
#include "src/track_entry_parser.h"
#include "webm/id.h"

namespace webm {

// Spec reference:
// http://matroska.org/technical/specs/index.html#Tracks
// http://www.webmproject.org/docs/container/#Tracks
class TracksParser : public MasterParser {
 public:
  TracksParser() : MasterParser(MakeChild<TrackEntryParser>(Id::kTrackEntry)) {}
};

}  // namespace webm

#endif  // SRC_TRACKS_PARSER_H_
