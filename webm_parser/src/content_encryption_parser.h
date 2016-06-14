#ifndef SRC_CONTENT_ENCRYPTION_PARSER_H_
#define SRC_CONTENT_ENCRYPTION_PARSER_H_

#include "src/byte_parser.h"
#include "src/content_enc_aes_settings_parser.h"
#include "src/master_value_parser.h"
#include "webm/dom_types.h"
#include "webm/id.h"

namespace webm {

// Spec reference:
// http://matroska.org/technical/specs/index.html#ContentEncryption
// http://www.webmproject.org/docs/container/#ContentEncryption
class ContentEncryptionParser : public MasterValueParser<ContentEncryption> {
 public:
  ContentEncryptionParser()
      : MasterValueParser<ContentEncryption>(
            MakeChild<IntParser<ContentEncAlgo>>(Id::kContentEncAlgo,
                                                 &ContentEncryption::algorithm),
            MakeChild<BinaryParser>(Id::kContentEncKeyId,
                                    &ContentEncryption::key_id),
            MakeChild<ContentEncAesSettingsParser>(
                Id::kContentEncAesSettings, &ContentEncryption::aes_settings)) {
  }
};

}  // namespace webm

#endif  // SRC_CONTENT_ENCRYPTION_PARSER_H_
