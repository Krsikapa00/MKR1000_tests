#pragma once

// Integer-to-text conversion without printf. Pure logic, no hardware: these
// functions can be unit-tested on the host (Step 10).
//
// Contract for every function:
//   - The output is always NUL-terminated when size > 0.
//   - The return value is the number of characters written, excluding the NUL.
//   - If buf is too small for the whole number, nothing is written except an
//     empty string (when size > 0) and the function returns 0. Partial numbers
//     are never produced, because a truncated number reads as a wrong number.

#include <cstddef>
#include <cstdint>

namespace util {

// Buffer sizes that always suffice, including the terminating NUL.
constexpr size_t kDecBufSize = 12;  // "-2147483648" is 11 chars
constexpr size_t kHexBufSize = 11;  // "0xFFFFFFFF" is 10 chars

size_t formatDec(uint32_t value, char* buf, size_t size);
size_t formatDec(int32_t value, char* buf, size_t size);

// Upper-case hex with a "0x" prefix and no leading zeros, e.g. 0xA0, 0x0.
size_t formatHex(uint32_t value, char* buf, size_t size);

}  // namespace util
