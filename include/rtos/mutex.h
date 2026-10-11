#pragma once

#include <FreeRTOS.h>
#include <semphr.h>

namespace rtos {

class Mutex {
public:
    Mutex() : handle_(xSemaphoreCreateMutexStatic(&storage_)) {}   // no heap
    Mutex(const Mutex&) = delete;
    Mutex& operator=(const Mutex&) = delete;

    // We want the user to always check the result since this could involve critical sections
    __attribute__((warn_unused_result))
    bool lock(TickType_t timeout = portMAX_DELAY) {
        configASSERT(!xPortIsInsideInterrupt());

        return xSemaphoreTake(handle_, timeout) == pdTRUE;
    }
    void unlock() { (void)xSemaphoreGive(handle_); }

private:
    StaticSemaphore_t storage_;
    SemaphoreHandle_t handle_;
};

} //namespace rtos