#ifndef SRC_BLOCK_ADDITIONS_PARSER_H_
#define SRC_BLOCK_ADDITIONS_PARSER_H_

#include "src/block_more_parser.h"
#include "src/master_value_parser.h"
#include "webm/dom_types.h"
#include "webm/id.h"

namespace webm {

// Spec reference:
// http://matroska.org/technical/specs/index.html#BlockAdditions
// http://www.webmproject.org/docs/container/#BlockAdditions
class BlockAdditionsParser : public MasterValueParser<BlockAdditions> {
 public:
  BlockAdditionsParser()
      : MasterValueParser<BlockAdditions>(MakeChild<BlockMoreParser>(
            Id::kBlockMore, &BlockAdditions::block_mores)) {}
};

}  // namespace webm

#endif  // SRC_BLOCK_ADDITIONS_PARSER_H_
