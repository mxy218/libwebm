#ifndef SRC_ID_PARSER_H_
#define SRC_ID_PARSER_H_

#include <cstdint>

#include "src/parser.h"
#include "webm/callback.h"
#include "webm/id.h"
#include "webm/reader.h"
#include "webm/status.h"

namespace webm {

// Parses an EBML ID from a byte stream.
class IdParser : public Parser {
 public:
  IdParser() = default;
  IdParser(IdParser&&) = default;
  IdParser& operator=(IdParser&&) = default;

  IdParser(const IdParser&) = delete;
  IdParser& operator=(const IdParser&) = delete;

  Status Feed(Callback* callback, Reader* reader,
              std::uint64_t* num_bytes_read) override;

  // Gets the parsed ID. This must not be called until the parse had been
  // successfully completed.
  Id id() const;

 private:
  int num_bytes_remaining_ = -1;
  Id id_;
};

}  // namespace webm

#endif  // SRC_ID_PARSER_H_
