#ifndef SRC_PARSER_H_
#define SRC_PARSER_H_

#include "webm/callback.h"
#include "webm/reader.h"
#include "webm/status.h"

namespace webm {

class Parser {
 public:
  virtual ~Parser() = default;

  // Feeds data into the parser, with the number of bytes read from the reader
  // returned in num_bytes_read. Returns Status::kOkCompleted when parsing is
  // complete, or an appropriate error code if the data is malformed and cannot
  // be parsed. Otherwise, the status of Reader::Read is returned if only a
  // partial parse could be done because the reader couldn't immediately provide
  // all the needed data. reader and num_bytes_read must not be null. Do not
  // call again once the parse is complete.
  virtual Status Feed(Callback* callback, Reader* reader,
                      std::uint64_t* num_bytes_read) = 0;
};

}  // namespace webm

#endif  // SRC_PARSER_H_
