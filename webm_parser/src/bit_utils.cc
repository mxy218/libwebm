#include "src/bit_utils.h"

#include <cstdint>

namespace webm {

std::uint8_t CountLeadingZeros(std::uint8_t value) {
  // Special case for 0 since we can't shift by sizeof(T) * 8 bytes.
  if (value == 0) return 8;

  std::uint8_t count = 0;
  while (!(value & (0x80 >> count))) {
    ++count;
  }
  return count;
}

}  // namespace webm
