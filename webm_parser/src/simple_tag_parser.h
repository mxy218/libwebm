#ifndef SRC_SIMPLE_TAG_PARSER_H_
#define SRC_SIMPLE_TAG_PARSER_H_

#include "src/bool_parser.h"
#include "src/byte_parser.h"
#include "src/master_value_parser.h"
#include "src/recursive_parser.h"
#include "webm/dom_types.h"
#include "webm/id.h"

namespace webm {

// Spec reference:
// http://matroska.org/technical/specs/index.html#SimpleTag
// http://www.webmproject.org/docs/container/#SimpleTag
class SimpleTagParser : public MasterValueParser<SimpleTag> {
 public:
  SimpleTagParser()
      : MasterValueParser<SimpleTag>(
            MakeChild<StringParser>(Id::kTagName, &SimpleTag::name),
            MakeChild<StringParser>(Id::kTagLanguage, &SimpleTag::language),
            MakeChild<BoolParser>(Id::kTagDefault, &SimpleTag::is_default),
            MakeChild<StringParser>(Id::kTagString, &SimpleTag::string),
            MakeChild<BinaryParser>(Id::kTagBinary, &SimpleTag::binary),
            MakeChild<RecursiveParser<SimpleTagParser>>(Id::kSimpleTag,
                                                        &SimpleTag::tags)) {}
};

}  // namespace webm

#endif  // SRC_SIMPLE_TAG_PARSER_H_
