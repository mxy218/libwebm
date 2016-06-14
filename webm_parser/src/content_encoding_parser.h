#ifndef SRC_CONTENT_ENCODING_PARSER_H_
#define SRC_CONTENT_ENCODING_PARSER_H_

#include "src/content_encryption_parser.h"
#include "src/int_parser.h"
#include "src/master_value_parser.h"
#include "webm/dom_types.h"
#include "webm/id.h"

namespace webm {

// Spec reference:
// http://matroska.org/technical/specs/index.html#ContentEncoding
// http://www.webmproject.org/docs/container/#ContentEncoding
class ContentEncodingParser : public MasterValueParser<ContentEncoding> {
 public:
  ContentEncodingParser()
      : MasterValueParser<ContentEncoding>(
            MakeChild<UnsignedIntParser>(Id::kContentEncodingOrder,
                                         &ContentEncoding::order),
            MakeChild<UnsignedIntParser>(Id::kContentEncodingScope,
                                         &ContentEncoding::scope),
            MakeChild<IntParser<ContentEncodingType>>(Id::kContentEncodingType,
                                                      &ContentEncoding::type),
            MakeChild<ContentEncryptionParser>(Id::kContentEncryption,
                                               &ContentEncoding::encryption)) {}
};

}  // namespace webm

#endif  // SRC_CONTENT_ENCODING_PARSER_H_
