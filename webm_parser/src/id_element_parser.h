#ifndef SRC_ID_ELEMENT_PARSER_H_
#define SRC_ID_ELEMENT_PARSER_H_

#include <cstdint>

#include "src/element_parser.h"
#include "webm/callback.h"
#include "webm/reader.h"
#include "webm/status.h"

namespace webm {

class IdElementParser : public ElementParser {
 public:
  IdElementParser() = default;

  IdElementParser(IdElementParser&&) = default;
  IdElementParser& operator=(IdElementParser&&) = default;

  IdElementParser(const IdElementParser&) = delete;
  IdElementParser& operator=(const IdElementParser&) = delete;

  Status Init(const ElementMetadata& metadata, std::uint64_t max_size) override;

  Status Feed(Callback* callback, Reader* reader,
              std::uint64_t* num_bytes_read) override;

  // Gets the parsed Id. This must not be called until the parse had been
  // successfully completed.
  Id value() const {
    assert(num_bytes_remaining_ == 0);
    return value_;
  }

  // Gets the parsed Id. This must not be called until the parse had been
  // successfully completed.
  Id* mutable_value() {
    assert(num_bytes_remaining_ == 0);
    return &value_;
  }

 private:
  Id value_;
  int num_bytes_remaining_ = -1;
};

}  // namespace webm

#endif  // SRC_ID_ELEMENT_PARSER_H_
