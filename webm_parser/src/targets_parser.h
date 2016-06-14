#ifndef SRC_TARGETS_PARSER_H_
#define SRC_TARGETS_PARSER_H_

#include "src/byte_parser.h"
#include "src/int_parser.h"
#include "src/master_value_parser.h"
#include "webm/dom_types.h"
#include "webm/id.h"

namespace webm {

// Spec reference:
// http://matroska.org/technical/specs/index.html#Targets
// http://www.webmproject.org/docs/container/#Targets
class TargetsParser : public MasterValueParser<Targets> {
 public:
  TargetsParser()
      : MasterValueParser<Targets>(
            MakeChild<UnsignedIntParser>(Id::kTargetTypeValue,
                                         &Targets::type_value),
            MakeChild<StringParser>(Id::kTargetType, &Targets::type),
            MakeChild<UnsignedIntParser>(Id::kTagTrackUid,
                                         &Targets::track_uids)) {}
};

}  // namespace webm

#endif  // SRC_TARGETS_PARSER_H_
