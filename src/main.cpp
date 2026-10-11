// FreeRTOS blink for the Arduino MKR1000 (ATSAMD21G18A).

#include <sam.h>

#include <FreeRTOS.h>
#include <task.h>

#include <cstdint>

#include "hal/gpio.h"
#include "hal/uart.h"
#include "rtos/mutex.h"
#include "rtos/lock_guard.h"

#include "app/log.h"

static_assert(__cplusplus >= 201103L, "C++11 or newer required");

namespace {

constexpr uint32_t    task1StackWords = configMINIMAL_STACK_SIZE;
constexpr UBaseType_t task1Priority   = tskIDLE_PRIORITY + 1;
constexpr uint32_t    task2StackWords = configMINIMAL_STACK_SIZE;
constexpr UBaseType_t task2Priority   = tskIDLE_PRIORITY + 1;
constexpr uint32_t    reporterTaskStackWords = configMINIMAL_STACK_SIZE;
constexpr UBaseType_t reporterTaskPriority   = tskIDLE_PRIORITY + 1;

StackType_t  task1Stack[task1StackWords];
StaticTask_t task1Tcb;
StackType_t  task2Stack[task2StackWords];
StaticTask_t task2Tcb;
StackType_t  reportTaskStack[reporterTaskStackWords];
StaticTask_t reportTaskTcb;
TaskHandle_t reporterHandle;

using Mutex = rtos::Mutex;

Mutex    counterMutex;
uint32_t counter = 0;

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

void noMutexTask(void*) {
    // for (;;) {
        for (int i = 0; i < 10000; i++) {
            uint32_t tmp = counter;
            taskYIELD();
            counter = tmp + 1;
        }
        xTaskNotifyGive(reporterHandle);
    // }
    for (;;) {
        vTaskSuspend(nullptr);
    }
}

void mutexTask(void*) {
    for (int i = 0; i < 10000; i++) {
        rtos::LockGuard guard(counterMutex, portMAX_DELAY);
        uint32_t tmp = counter;
        taskYIELD();
        counter = tmp + 1;
    }
    xTaskNotifyGive(reporterHandle);

    for (;;) {
        vTaskSuspend(nullptr);
    }
}

void reporterTask(void*) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    app::Log::line("counter=", counter, " expected=", 20000);

    for (;;) {
        vTaskSuspend(nullptr);
    }
}

// Something has gone fatally wrong: stop everything and hold the LED on.
[[noreturn]] void halt() {
    taskDISABLE_INTERRUPTS();

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
    app::Log::init();

    xTaskCreateStatic(noMutexTask, "task1", task1StackWords, nullptr, task1Priority, task1Stack,
                      &task1Tcb);
    xTaskCreateStatic(noMutexTask, "task2", task2StackWords, nullptr, task2Priority, task2Stack,
                      &task2Tcb);
    reporterHandle = xTaskCreateStatic(reporterTask, "reporterTask", reporterTaskStackWords, nullptr, reporterTaskPriority, reportTaskStack,
                      &reportTaskTcb);


    vTaskStartScheduler();  // does not return while the scheduler runs
    halt();
}
