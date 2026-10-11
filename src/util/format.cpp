#include "util/format.h"

namespace util {

namespace {

// Writes digits of `value` in `base` (2..16), most significant first, after an
// optional prefix. Digits are produced least significant first into a scratch
// buffer, then copied out in the right order.
size_t formatUnsigned(uint32_t value, uint32_t base, const char* prefix, char* buf,
                      size_t size) {
    static const char kDigits[] = "0123456789ABCDEF";

    char scratch[32];  // enough for 32 binary digits; decimal and hex need far fewer
    size_t digits = 0;
    do {
        scratch[digits++] = kDigits[value % base];
        value /= base;
    } while (value != 0);

    size_t prefixLen = 0;
    while (prefix[prefixLen] != '\0') {
        ++prefixLen;
    }

    const size_t total = prefixLen + digits;
    if (total + 1 > size) {  // +1 for the NUL
        if (size > 0) {
            buf[0] = '\0';
        }
        return 0;
    }

    size_t out = 0;
    for (size_t i = 0; i < prefixLen; ++i) {
        buf[out++] = prefix[i];
    }
    while (digits > 0) {
        buf[out++] = scratch[--digits];
    }
    buf[out] = '\0';
    return out;
}

}  // namespace

size_t formatDec(uint32_t value, char* buf, size_t size) {
    return formatUnsigned(value, 10, "", buf, size);
}

size_t formatDec(int32_t value, char* buf, size_t size) {
    if (value >= 0) {
        return formatUnsigned(static_cast<uint32_t>(value), 10, "", buf, size);
    }
    // -INT32_MIN overflows int32_t (undefined behaviour). Unsigned arithmetic is
    // defined to wrap, so 0u - value gives the magnitude for every negative value.
    const uint32_t magnitude = 0u - static_cast<uint32_t>(value);
    return formatUnsigned(magnitude, 10, "-", buf, size);
}

size_t formatHex(uint32_t value, char* buf, size_t size) {
    return formatUnsigned(value, 16, "0x", buf, size);
}

}  // namespace util
