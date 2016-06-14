#ifndef SRC_TIME_SLICE_PARSER_H_
#define SRC_TIME_SLICE_PARSER_H_

#include "src/int_parser.h"
#include "src/master_value_parser.h"
#include "webm/dom_types.h"
#include "webm/id.h"

namespace webm {

// Spec reference:
// http://matroska.org/technical/specs/index.html#TimeSlice
// http://www.webmproject.org/docs/container/#TimeSlice
class TimeSliceParser : public MasterValueParser<TimeSlice> {
 public:
  TimeSliceParser()
      : MasterValueParser<TimeSlice>(MakeChild<UnsignedIntParser>(
            Id::kLaceNumber, &TimeSlice::lace_number)) {}
};

}  // namespace webm

#endif  // SRC_TIME_SLICE_PARSER_H_
