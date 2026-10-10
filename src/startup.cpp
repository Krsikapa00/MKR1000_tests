// Minimal startup for the ATSAMD21G18A: vector table, RAM initialisation,
// static constructors, then main().

#include <sam.h>

#include <cstdint>

extern "C" {

// Symbols provided by ld/samd21g18a_app.ld
extern uint32_t __stack_top;
extern uint32_t __data_load;
extern uint32_t __data_start;
extern uint32_t __data_end;
extern uint32_t __bss_start;
extern uint32_t __bss_end;
extern void (*__init_array_start[])();
extern void (*__init_array_end[])();

int main();

void Reset_Handler();
void Default_Handler();

// Unused handlers fall through to Default_Handler unless overridden elsewhere.
void NMI_Handler() __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler() __attribute__((weak, alias("Default_Handler")));
void SVC_Handler() __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler() __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler() __attribute__((weak, alias("Default_Handler")));

}  // extern "C"

namespace {

using Vector = void (*)();

// Core exceptions only; peripheral IRQ slots will be added as they are needed.
__attribute__((section(".vectors"), used))
const Vector kVectorTable[] = {
    reinterpret_cast<Vector>(&__stack_top),
    Reset_Handler,
    NMI_Handler,
    HardFault_Handler,
    nullptr,  // reserved
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    SVC_Handler,
    nullptr,  // reserved
    nullptr,
    PendSV_Handler,
    SysTick_Handler,
};

}  // namespace

extern "C" void Reset_Handler() {
    // The bootloader jumps here; make sure exceptions use our table.
    SCB->VTOR = reinterpret_cast<uint32_t>(kVectorTable) & SCB_VTOR_TBLOFF_Msk;

    const uint32_t* src = &__data_load;
    for (uint32_t* dst = &__data_start; dst < &__data_end;) {
        *dst++ = *src++;
    }
    for (uint32_t* dst = &__bss_start; dst < &__bss_end;) {
        *dst++ = 0;
    }
    for (auto ctor = __init_array_start; ctor < __init_array_end; ++ctor) {
        (*ctor)();
    }

    main();
    for (;;) {
    }
}

extern "C" void Default_Handler() {
    for (;;) {
    }
}
