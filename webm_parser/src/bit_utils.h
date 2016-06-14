#ifndef SRC_BIT_UTILS_H_
#define SRC_BIT_UTILS_H_

#include <cstdint>

namespace webm {

// Counts the number of leading zero bits.
// For example:
//   assert(8 == CountLeadingZeros(0x00));
//   assert(4 == CountLeadingZeros(0x0f));
//   assert(0 == CountLeadingZeros(0xf0));
std::uint8_t CountLeadingZeros(std::uint8_t value);

}  // namespace webm

#endif  // SRC_BIT_UTILS_H_
