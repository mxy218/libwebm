#ifndef SRC_VAR_INT_PARSER_H_
#define SRC_VAR_INT_PARSER_H_

#include <cassert>
#include <cstdint>

#include "src/parser.h"
#include "webm/callback.h"
#include "webm/reader.h"
#include "webm/status.h"

namespace webm {

class VarIntParser : public Parser {
 public:
  VarIntParser() = default;
  VarIntParser(VarIntParser&&) = default;
  VarIntParser& operator=(VarIntParser&&) = default;

  VarIntParser(const VarIntParser&) = delete;
  VarIntParser& operator=(const VarIntParser&) = delete;

  Status Feed(Callback* callback, Reader* reader,
              std::uint64_t* num_bytes_read) override;

  // Gets the parsed value. This must not be called until the parse had been
  // successfully completed.
  std::uint64_t value() const {
    assert(num_bytes_remaining_ == 0);
    return value_;
  }

  // Gets the number of bytes which were used to encode the integer value in the
  // byte stream. This must not be called until the parse had been successfully
  // completed.
  int encoded_length() const {
    assert(num_bytes_remaining_ == 0);
    return total_data_bytes_ + 1;
  }

 private:
  int num_bytes_remaining_ = -1;
  int total_data_bytes_;
  std::uint64_t value_;
};

}  // namespace webm

#endif  // SRC_VAR_INT_PARSER_H_
