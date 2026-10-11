#pragma once

// Thread-safe line logger over the debug UART (SERCOM5, pins 13/14, 115200 8N1).
//
//   app::Log::line("chip id=", app::hex(0xA0), " retries=", 3, " ok=", true);
//
// Every call writes one complete line ending in "\r\n". While the scheduler is
// running, the whole line is written under a mutex, so lines from different tasks
// never interleave. Must not be called from an interrupt handler.
//
// Supported argument types: const char* / string literals, char, bool, any
// integer type up to 32 bits (printed in decimal), and app::Hex. Anything else
// (float, pointers, 64-bit integers) is a compile error rather than a surprise.

#include <cstdint>
#include <type_traits>

#include <FreeRTOS.h>
#include <task.h>

#include "hal/uart.h"
#include "rtos/lock_guard.h"
#include "rtos/mutex.h"

namespace app {

// Wrapper that selects hex output: Log::line("reg=", app::hex(value)).
struct Hex {
    uint32_t value;
};

inline Hex hex(uint32_t value) {
    return Hex{value};
}

class Log {
public:
    using Uart = hal::Sercom5Uart<115200>;

    Log() = delete;

    // Brings up the UART. Call once from main() before any task logs.
    static void init();

    template <typename... Args>
    static void line(const Args&... args) {
        if (xTaskGetSchedulerState() == taskSCHEDULER_RUNNING) {
            rtos::LockGuard guard(mutex_, portMAX_DELAY);
            configASSERT(guard.ownsLock());  // portMAX_DELAY cannot time out
            writeLine(args...);
        } else {
            // Before the scheduler starts there is only one thread, and FreeRTOS
            // mutexes must not be taken yet.
            writeLine(args...);
        }
    }

private:
    template <typename... Args>
    static void writeLine(const Args&... args) {
        writeAll(args...);
        put("\r\n");
    }

    // Variadic recursion: write the first argument, then the rest.
    template <typename T, typename... Rest>
    static void writeAll(const T& first, const Rest&... rest) {
        put(first);
        writeAll(rest...);
    }
    static void writeAll() {}  // base case: nothing left

    // Non-template overloads are preferred over the integer templates below on an
    // exact match, so char prints as a character and bool as true/false.
    static void put(const char* str);
    static void put(char ch);
    static void put(bool value);
    static void put(Hex value);
    static void putSigned(int32_t value);
    static void putUnsigned(uint32_t value);

    // On this toolchain int32_t is `long`, so separate int32_t/uint32_t overloads
    // would make put(42) (an `int`) ambiguous. These templates route every
    // integer type of 32 bits or less to the right formatter instead.
    template <typename T>
    static typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value &&
                                   sizeof(T) <= sizeof(int32_t)>::type
    put(T value) {
        putSigned(static_cast<int32_t>(value));
    }

    template <typename T>
    static typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value &&
                                   sizeof(T) <= sizeof(uint32_t)>::type
    put(T value) {
        putUnsigned(static_cast<uint32_t>(value));
    }

    // Explicitly rejected, so misuse is a clear "use of deleted function" error:
    // floats would otherwise be an ambiguous call, and any pointer would
    // silently convert to bool and print "true".
    static void put(float) = delete;
    static void put(double) = delete;
    template <typename T>
    static void put(const T*) = delete;

    static rtos::Mutex mutex_;
};

}  // namespace app
