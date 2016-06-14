#ifndef SRC_UNKNOWN_PARSER_H_
#define SRC_UNKNOWN_PARSER_H_

#include <cstdint>

#include "src/element_parser.h"
#include "webm/callback.h"
#include "webm/reader.h"
#include "webm/status.h"

namespace webm {

// Parses unknown elements by delegating to Callback::OnUnknownElement.
class UnknownParser : public ElementParser {
 public:
  Status Init(const ElementMetadata& metadata, std::uint64_t max_size) override;

  Status Feed(Callback* callback, Reader* reader,
              std::uint64_t* num_bytes_read) override;

 private:
  // The metadata for this element.
  ElementMetadata metadata_;

  // The number of bytes remaining that have not been read in the element.
  std::uint64_t bytes_remaining_;
};

}  // namespace webm

#endif  // SRC_UNKNOWN_PARSER_H_
