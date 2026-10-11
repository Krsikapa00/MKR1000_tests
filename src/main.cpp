// FreeRTOS blink for the Arduino MKR1000 (ATSAMD21G18A).

#include <sam.h>

#include <FreeRTOS.h>
#include <task.h>

#include <cstdint>

#include "hal/gpio.h"
#include "hal/uart.h"

static_assert(__cplusplus >= 201103L, "C++11 or newer required");

namespace {

constexpr uint32_t kBlinkHalfPeriodMs = 1000;
constexpr uint32_t kBlinkStackWords = configMINIMAL_STACK_SIZE;
constexpr UBaseType_t kBlinkPriority = tskIDLE_PRIORITY + 1;

using UsableLed = hal::Led<hal::OutputPin<0, 20>, true>;  // PA20
using TestInput = hal::InputPin<0, 22>;
using Uart5 = hal::Sercom5Uart<115200>;
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
    UsableLed::init();
    TestInput::init(hal::Pull::Up);

    // xTaskDelayUntil keeps a fixed period regardless of how long the body takes.
    TickType_t lastWake = xTaskGetTickCount();
    for (;;) {
        if (TestInput::read()) {
            UsableLed::toggle();
        }

        xTaskDelayUntil(&lastWake, pdMS_TO_TICKS(kBlinkHalfPeriodMs));
    }
}

StackType_t blinkStack[kBlinkStackWords];
StaticTask_t blinkTcb;

// Something has gone fatally wrong: stop everything and hold the LED on.
[[noreturn]] void halt() {
    taskDISABLE_INTERRUPTS();
    UsableLed::init();
    UsableLed::on();
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
    Uart5::init();

    xTaskCreateStatic(blinkTask, "blink", kBlinkStackWords, nullptr, kBlinkPriority, blinkStack,
                      &blinkTcb);

    vTaskStartScheduler();  // does not return while the scheduler runs
    halt();
}
