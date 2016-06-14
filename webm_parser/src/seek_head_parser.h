#ifndef SRC_SEEK_HEAD_PARSER_H_
#define SRC_SEEK_HEAD_PARSER_H_

#include "src/master_parser.h"
#include "src/seek_parser.h"
#include "webm/id.h"

namespace webm {

// Spec reference:
// http://matroska.org/technical/specs/index.html#SeekHead
// http://www.webmproject.org/docs/container/#SeekHead
class SeekHeadParser : public MasterParser {
 public:
  SeekHeadParser() : MasterParser(MakeChild<SeekParser>(Id::kSeek)) {}
};

}  // namespace webm

#endif  // SRC_SEEK_HEAD_PARSER_H_
