#ifndef SRC_CUES_PARSER_H_
#define SRC_CUES_PARSER_H_

#include "src/cue_point_parser.h"
#include "src/master_parser.h"
#include "webm/id.h"

namespace webm {

// Spec reference:
// http://matroska.org/technical/specs/index.html#Cues
// http://www.webmproject.org/docs/container/#Cues
class CuesParser : public MasterParser {
 public:
  CuesParser() : MasterParser(MakeChild<CuePointParser>(Id::kCuePoint)) {}
};

}  // namespace webm

#endif  // SRC_CUES_PARSER_H_
