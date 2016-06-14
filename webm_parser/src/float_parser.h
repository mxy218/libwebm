#ifndef SRC_FLOAT_PARSER_H_
#define SRC_FLOAT_PARSER_H_

#include <cassert>
#include <cstdint>

#include "src/element_parser.h"
#include "webm/callback.h"
#include "webm/element.h"
#include "webm/reader.h"
#include "webm/status.h"

namespace webm {

// Parses an EBML float from a byte stream.
class FloatParser : public ElementParser {
 public:
  // Constructs a new parser which will use the given default_value as the
  // value for the element if its size is zero. Defaults to the value zero (as
  // the EBML spec indicates).
  explicit FloatParser(double default_value = 0.0);

  FloatParser(FloatParser&&) = default;
  FloatParser& operator=(FloatParser&&) = default;

  FloatParser(const FloatParser&) = delete;
  FloatParser& operator=(const FloatParser&) = delete;

  Status Init(const ElementMetadata& metadata, std::uint64_t max_size) override;

  Status Feed(Callback* callback, Reader* reader,
              std::uint64_t* num_bytes_read) override;

  // Gets the parsed float. This must not be called until the parse had been
  // successfully completed.
  double value() const {
    assert(num_bytes_remaining_ == 0);
    return value_;
  }

  // Gets the parsed float. This must not be called until the parse had been
  // successfully completed.
  double* mutable_value() {
    assert(num_bytes_remaining_ == 0);
    return &value_;
  }

 private:
  double value_;
  double default_value_;
  std::uint64_t uint64_value_;
  int num_bytes_remaining_ = -1;
  bool use_32_bits_;
};

}  // namespace webm

#endif  // SRC_FLOAT_PARSER_H_
