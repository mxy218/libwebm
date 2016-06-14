#ifndef SRC_TAG_PARSER_H_
#define SRC_TAG_PARSER_H_

#include "src/master_value_parser.h"
#include "src/simple_tag_parser.h"
#include "src/targets_parser.h"
#include "webm/dom_types.h"
#include "webm/id.h"

namespace webm {

// Spec reference:
// http://matroska.org/technical/specs/index.html#Tag
// http://www.webmproject.org/docs/container/#Tag
class TagParser : public MasterValueParser<Tag> {
 public:
  TagParser()
      : MasterValueParser<Tag>(
            MakeChild<TargetsParser>(Id::kTargets, &Tag::targets),
            MakeChild<SimpleTagParser>(Id::kSimpleTag, &Tag::tags)) {}

 protected:
  Status OnParseCompleted(Callback* callback) override {
    return callback->OnTag(metadata(Id::kTag), value());
  }
};

}  // namespace webm

#endif  // SRC_TAG_PARSER_H_
