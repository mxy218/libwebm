#ifndef SRC_TAGS_PARSER_H_
#define SRC_TAGS_PARSER_H_

#include "src/master_parser.h"
#include "src/tag_parser.h"
#include "webm/id.h"

namespace webm {

// Spec reference:
// http://matroska.org/technical/specs/index.html#Tags
// http://www.webmproject.org/docs/container/#Tags
class TagsParser : public MasterParser {
 public:
  TagsParser() : MasterParser(MakeChild<TagParser>(Id::kTag)) {}
};

}  // namespace webm

#endif  // SRC_TAGS_PARSER_H_
