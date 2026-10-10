// FreeRTOS blink for the Arduino MKR1000 (ATSAMD21G18A).
// One task toggles the on-board LED (PA20) every 500 ms: a 1 Hz blink.

#include <sam.h>

#include <FreeRTOS.h>
#include <task.h>

#include <cstdint>

static_assert(__cplusplus >= 201103L, "C++11 or newer required");

namespace {

constexpr uint32_t kBlinkHalfPeriodMs = 100;  // 500 ms on + 500 ms off = 1 Hz
constexpr uint32_t kBlinkStackWords = configMINIMAL_STACK_SIZE;
constexpr UBaseType_t kBlinkPriority = tskIDLE_PRIORITY + 1;

// A GPIO output on a fixed port/pin, resolved entirely at compile time.
template <uint8_t PortNum, uint8_t PinNum>
class OutputPin {
    static_assert(PortNum < 2, "SAMD21G has ports A (0) and B (1)");
    static_assert(PinNum < 32, "Pin number out of range");

    static constexpr uint32_t kMask = 1ul << PinNum;

public:
    static void init() { PORT->Group[PortNum].DIRSET.reg = kMask; }
    static void set() { PORT->Group[PortNum].OUTSET.reg = kMask; }
    static void clear() { PORT->Group[PortNum].OUTCLR.reg = kMask; }
    static void toggle() { PORT->Group[PortNum].OUTTGL.reg = kMask; }
};

using Led = OutputPin<0, 20>;  // PA20

// The bootloader may leave the CPU on the 48 MHz DFLL; put it in the state
// FreeRTOSConfig.h assumes (configCPU_CLOCK_HZ).
void initClock() {
    SYSCTRL->OSC8M.bit.PRESC = 0;  // 8 MHz / 1
    SYSCTRL->OSC8M.bit.ENABLE = 1;
    while (!SYSCTRL->PCLKSR.bit.OSC8MRDY) {
    }

    GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(0) | GCLK_GENCTRL_SRC_OSC8M | GCLK_GENCTRL_GENEN;
    while (GCLK->STATUS.bit.SYNCBUSY) {
    }
}

void blinkTask(void*) {
    Led::init();

    // xTaskDelayUntil keeps a fixed period regardless of how long the body takes.
    TickType_t lastWake = xTaskGetTickCount();
    for (;;) {
        Led::toggle();
        xTaskDelayUntil(&lastWake, pdMS_TO_TICKS(kBlinkHalfPeriodMs));
    }
}

StackType_t blinkStack[kBlinkStackWords];
StaticTask_t blinkTcb;

// Something has gone fatally wrong: stop everything and hold the LED on.
[[noreturn]] void halt() {
    taskDISABLE_INTERRUPTS();
    Led::init();
    Led::set();
    for (;;) {
    }
}

}  // namespace

extern "C" {

void vAssertCalled(const char* /*file*/, int /*line*/) {
    halt();
}

void vApplicationMallocFailedHook() {
    halt();
}

void vApplicationStackOverflowHook(TaskHandle_t /*task*/, char* /*name*/) {
    halt();
}

}  // extern "C"

int main() {
    initClock();

    xTaskCreateStatic(blinkTask, "blink", kBlinkStackWords, nullptr, kBlinkPriority, blinkStack,
                      &blinkTcb);

    vTaskStartScheduler();  // does not return while the scheduler runs
    halt();
}
