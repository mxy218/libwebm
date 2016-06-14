#ifndef SRC_BLOCK_MORE_PARSER_H_
#define SRC_BLOCK_MORE_PARSER_H_

#include "src/byte_parser.h"
#include "src/int_parser.h"
#include "src/master_value_parser.h"
#include "webm/dom_types.h"
#include "webm/id.h"

namespace webm {

// Spec reference:
// http://matroska.org/technical/specs/index.html#BlockMore
// http://www.webmproject.org/docs/container/#BlockMore
class BlockMoreParser : public MasterValueParser<BlockMore> {
 public:
  BlockMoreParser()
      : MasterValueParser<BlockMore>(
            MakeChild<UnsignedIntParser>(Id::kBlockAddId, &BlockMore::id),
            MakeChild<BinaryParser>(Id::kBlockAdditional, &BlockMore::data)) {}
};

}  // namespace webm

#endif  // SRC_BLOCK_MORE_PARSER_H_
