#ifndef SRC_SIZE_PARSER_H_
#define SRC_SIZE_PARSER_H_

#include <cstdint>

#include "src/parser.h"
#include "src/var_int_parser.h"
#include "webm/callback.h"
#include "webm/reader.h"
#include "webm/status.h"

namespace webm {

class SizeParser : public Parser {
 public:
  SizeParser() = default;
  SizeParser(SizeParser&&) = default;
  SizeParser& operator=(SizeParser&&) = default;

  SizeParser(const SizeParser&) = delete;
  SizeParser& operator=(const SizeParser&) = delete;

  Status Feed(Callback* callback, Reader* reader,
              std::uint64_t* num_bytes_read) override;

  // Gets the parsed size. This must not be called until the parse had been
  // successfully completed.
  std::uint64_t size() const;

 private:
  VarIntParser uint_parser_;
};

}  // namespace webm

#endif  // SRC_SIZE_PARSER_H_
