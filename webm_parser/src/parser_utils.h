#ifndef SRC_PARSER_UTILS_H_
#define SRC_PARSER_UTILS_H_

#include <cassert>
#include <cstdint>
#include <type_traits>

#include "webm/reader.h"
#include "webm/status.h"

namespace webm {

// Reads a single byte from the reader, and returns the status of the read. If
// the status is not Status::kOkCompleted, then no data was read.
Status ReadByte(Reader* reader, std::uint8_t* byte);

// Accumulates bytes from the reader into the integer. The integer will be
// extracted as a big-endian integer and stored with the native
// host-endianness. num_bytes_remaining is the number of bytes to
template <typename T>
Status AccumulateIntegerBytes(int num_to_read, Reader* reader, T* integer,
                              std::uint64_t* num_actually_read) {
  static_assert(std::is_integral<T>::value || std::is_enum<T>::value,
                "T must be an integer or enum type");
  // Use unsigned integers for bitwise arithmetic because it's well-defined (as
  // opposed to signed integers, where left shifting a negative integer is
  // undefined, for example).
  using UnsignedT = typename std::make_unsigned<T>::type;

  assert(reader != nullptr);
  assert(integer != nullptr);
  assert(num_actually_read != nullptr);
  assert(num_to_read >= 0);
  assert(num_to_read <= sizeof(T));

  *num_actually_read = 0;

  if (num_to_read < 0 || num_to_read > sizeof(T)) {
    return Status(Status::kInvalidElementSize);
  }

  for (; num_to_read > 0; --num_to_read) {
    std::uint8_t byte;
    const Status status = ReadByte(reader, &byte);
    if (!status.completed_ok()) {
      return status;
    }
    ++*num_actually_read;
    *integer = static_cast<T>((static_cast<UnsignedT>(*integer) << 8) | byte);
  }

  return Status(Status::kOkCompleted);
}

}  // namespace webm

#endif  // SRC_PARSER_UTILS_H_
