#ifndef SRC_CONTENT_ENCODINGS_PARSER_H_
#define SRC_CONTENT_ENCODINGS_PARSER_H_

#include "src/content_encoding_parser.h"
#include "src/master_value_parser.h"
#include "webm/dom_types.h"
#include "webm/id.h"

namespace webm {

// Spec reference:
// http://matroska.org/technical/specs/index.html#ContentEncodings
// http://www.webmproject.org/docs/container/#ContentEncodings
class ContentEncodingsParser : public MasterValueParser<ContentEncodings> {
 public:
  ContentEncodingsParser()
      : MasterValueParser<ContentEncodings>(MakeChild<ContentEncodingParser>(
            Id::kContentEncoding, &ContentEncodings::encodings)) {}
};

}  // namespace webm

#endif  // SRC_CONTENT_ENCODINGS_PARSER_H_
