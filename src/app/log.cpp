#include "app/log.h"

#include "util/format.h"

namespace app {

// Constructed before main() by the startup code's constructor loop. Creating a
// static FreeRTOS mutex needs neither the heap nor a running scheduler.
rtos::Mutex Log::mutex_;

void Log::init() {
    Uart::init();
}

void Log::put(const char* str) {
    Uart::write(str);
}

void Log::put(char ch) {
    Uart::write(ch);
}

void Log::put(bool value) {
    Uart::write(value ? "true" : "false");
}

void Log::put(Hex value) {
    char buf[util::kHexBufSize];
    util::formatHex(value.value, buf, sizeof buf);
    Uart::write(buf);
}

void Log::putSigned(int32_t value) {
    char buf[util::kDecBufSize];
    util::formatDec(value, buf, sizeof buf);
    Uart::write(buf);
}

void Log::putUnsigned(uint32_t value) {
    char buf[util::kDecBufSize];
    util::formatDec(value, buf, sizeof buf);
    Uart::write(buf);
}

}  // namespace app
