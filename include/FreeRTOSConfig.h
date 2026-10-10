// FreeRTOS kernel configuration for the Arduino MKR1000 (ATSAMD21G18A, Cortex-M0+).
// Reference: https://www.freertos.org/a00110.html

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

// ---- Hardware --------------------------------------------------------------
#define configCPU_CLOCK_HZ                      8000000UL  // OSC8M, set in main.cpp
#define configTICK_RATE_HZ                      1000
#define configTICK_TYPE_WIDTH_IN_BITS           TICK_TYPE_WIDTH_32_BITS

// ARMv6-M port options: no MPU, FPU or TrustZone on this part.
#define configENABLE_MPU                        0
#define configENABLE_FPU                        0
#define configENABLE_TRUSTZONE                  0
#define configRUN_FREERTOS_SECURE_ONLY          0

// ---- Scheduler -------------------------------------------------------------
#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0  // not available on Cortex-M0
#define configUSE_TICKLESS_IDLE                 0
#define configMAX_PRIORITIES                    5
#define configMINIMAL_STACK_SIZE                128  // words
#define configMAX_TASK_NAME_LEN                 12
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   1

// ---- Memory ----------------------------------------------------------------
#define configSUPPORT_STATIC_ALLOCATION         1
#define configSUPPORT_DYNAMIC_ALLOCATION        1
#define configKERNEL_PROVIDED_STATIC_MEMORY     1  // kernel supplies idle/timer task memory
#define configTOTAL_HEAP_SIZE                   (8 * 1024)  // heap_4

// ---- Synchronisation primitives --------------------------------------------
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           1
#define configQUEUE_REGISTRY_SIZE               0

// ---- Software timers -------------------------------------------------------
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH                5
#define configTIMER_TASK_STACK_DEPTH            configMINIMAL_STACK_SIZE

// ---- Hooks and debugging ---------------------------------------------------
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_MALLOC_FAILED_HOOK            1
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_TRACE_FACILITY                0
#define configGENERATE_RUN_TIME_STATS           0

#ifdef __cplusplus
extern "C" {
#endif
void vAssertCalled(const char* file, int line);
#ifdef __cplusplus
}
#endif
#define configASSERT(x)                                                                            \
    do {                                                                                           \
        if ((x) == 0) {                                                                            \
            vAssertCalled(__FILE__, __LINE__);                                                     \
        }                                                                                          \
    } while (0)

// ---- Optional API functions ------------------------------------------------
#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskDelayUntil                 1
#define INCLUDE_vTaskDelete                     1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_uxTaskPriorityGet               1
#define INCLUDE_vTaskPrioritySet                1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_xTaskGetSchedulerState          1

#endif  // FREERTOS_CONFIG_H
