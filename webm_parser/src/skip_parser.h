#ifndef SRC_SKIP_PARSER_H_
#define SRC_SKIP_PARSER_H_

#include <cstdint>

#include "src/element_parser.h"
#include "webm/callback.h"
#include "webm/reader.h"
#include "webm/status.h"

namespace webm {

// A simple parser that merely skips (via Reader::Skip) ahead in the stream
// until the element has been fully skipped.
class SkipParser : public ElementParser {
 public:
  Status Init(const ElementMetadata& metadata, std::uint64_t max_size) override;

  Status Feed(Callback* callback, Reader* reader,
              std::uint64_t* num_bytes_read) override;

 private:
  std::uint64_t num_bytes_remaining_;
};

}  // namespace webm

#endif  // SRC_SKIP_PARSER_H_
