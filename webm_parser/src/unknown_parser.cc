#include "src/unknown_parser.h"

#include <cassert>
#include <cstdint>

#include "webm/element.h"
#include "webm/reader.h"
#include "webm/status.h"

namespace webm {

Status UnknownParser::Init(const ElementMetadata& metadata,
                           std::uint64_t max_size) {
  assert(metadata.size == kUnknownElementSize || metadata.size <= max_size);

  if (metadata.size == kUnknownElementSize) {
    return Status(Status::kIndefiniteUnknownElement);
  }

  metadata_ = metadata;
  bytes_remaining_ = metadata.size;

  return Status(Status::kOkCompleted);
}

Status UnknownParser::Feed(Callback* callback, Reader* reader,
                           std::uint64_t* num_bytes_read) {
  assert(callback != nullptr);
  assert(reader != nullptr);
  assert(num_bytes_read != nullptr);

  const std::uint64_t original_bytes_remaining = bytes_remaining_;
  const Status status =
      callback->OnUnknownElement(metadata_, reader, &bytes_remaining_);
  assert(bytes_remaining_ <= original_bytes_remaining);

  *num_bytes_read = original_bytes_remaining - bytes_remaining_;

  return status;
}

}  // namespace webm
