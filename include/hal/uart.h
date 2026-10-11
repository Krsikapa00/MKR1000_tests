#pragma once

#include <sam.h>
#include <cstddef>
#include <cstdint>
#include "gpio.h"
#include "clock.h"

// Design choice: the HAL uses the FreeRTOS tick for read timeouts. Everything in this
// project runs under FreeRTOS; if the HAL ever needs to run without it (e.g. host tests
// in Step 10), wrap the tick behind a small nowMs() function instead.
#include <FreeRTOS.h>
#include <task.h>

namespace hal {

enum class UartStatus : uint8_t { Ok, Timeout, FrameError, Overflow };

constexpr uint32_t usartBaud(uint32_t baud, uint32_t refClk) {
    return 65536u - static_cast<uint32_t>((static_cast<uint64_t>(baud) << 20) / refClk);
}

template<uint32_t BaudRate, uint32_t RefClk = kCpuHz>
class Sercom5Uart {
    static_assert(BaudRate > 0, "Baud Rate cannot be 0");
    static_assert(BaudRate * 16u <= RefClk, "Ref clk must be 16x greater than Baud");
    static_assert(usartBaud(BaudRate, RefClk) <= 0xFFFF, "BAUD Must fit within 16 bits");

    static constexpr uint32_t kBaudReg = usartBaud(BaudRate, RefClk);
public:
    Sercom5Uart() = delete;
    // SERCOM5 Configuration
    static void init() {
        hal::PeripheralPin<1, 22>::init(hal::PeriphFunction::D);
        hal::PeripheralPin<1, 23>::init(hal::PeriphFunction::D);
        PM->APBCMASK.reg |= PM_APBCMASK_SERCOM5;
        GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN
                          | GCLK_CLKCTRL_GEN_GCLK0
                          | GCLK_CLKCTRL_ID_SERCOM5_CORE;

        while (GCLK->STATUS.bit.SYNCBUSY){
        }

        SERCOM5->USART.CTRLA.reg = SERCOM_USART_CTRLA_SWRST;
        while (SERCOM5->USART.SYNCBUSY.bit.SWRST){}

        SERCOM5->USART.CTRLA.reg = SERCOM_USART_CTRLA_DORD
                                 | SERCOM_USART_CTRLA_TXPO(1)
                                 | SERCOM_USART_CTRLA_RXPO(3)
                                 | SERCOM_USART_CTRLA_MODE_USART_INT_CLK;

        SERCOM5->USART.BAUD.bit.BAUD = kBaudReg;

        SERCOM5->USART.CTRLB.reg = SERCOM_USART_CTRLB_RXEN
                                 | SERCOM_USART_CTRLB_TXEN;
        while (SERCOM5->USART.SYNCBUSY.bit.CTRLB){}

        SERCOM5->USART.CTRLA.reg |= SERCOM_USART_CTRLA_ENABLE;
        while (SERCOM5->USART.SYNCBUSY.bit.ENABLE){}

    }

    static void write(char ch) {
        while(!SERCOM5->USART.INTFLAG.bit.DRE) {}
        SERCOM5->USART.DATA.reg = ch;
    }

    static void write(const char *str) {
        while (*str != '\0') {
            write(*str++);
        }
    }

    // True if a received byte is waiting; never blocks.
    static bool available() {
        return SERCOM5->USART.INTFLAG.bit.RXC;
    }

    // Receives one byte. `out` is written only when the result is UartStatus::Ok.
    // Polls for up to timeoutMs: at 115200 baud a byte can arrive every ~87 us, far
    // faster than a 1 ms vTaskDelay, so this spins (Step 8 replaces it with an interrupt).
    __attribute__((warn_unused_result))
    static UartStatus read(uint8_t& out, uint32_t timeoutMs) {
        const TickType_t start = xTaskGetTickCount();
        while (!SERCOM5->USART.INTFLAG.bit.RXC) {
            // Unsigned subtraction gives the right elapsed time even when the tick count wraps.
            if (xTaskGetTickCount() - start >= pdMS_TO_TICKS(timeoutMs)) {
                return UartStatus::Timeout;
            }
        }

        // STATUS describes the byte now in DATA, so read it before DATA.
        const uint16_t status = SERCOM5->USART.STATUS.reg;
        const uint8_t data = static_cast<uint8_t>(SERCOM5->USART.DATA.reg);  // also clears RXC

        if (status & SERCOM_USART_STATUS_BUFOVF) {
            SERCOM5->USART.STATUS.reg = SERCOM_USART_STATUS_BUFOVF;  // write 1 to clear
            return UartStatus::Overflow;
        }
        if (status & SERCOM_USART_STATUS_FERR) {
            SERCOM5->USART.STATUS.reg = SERCOM_USART_STATUS_FERR;  // write 1 to clear
            return UartStatus::FrameError;
        }

        out = data;
        return UartStatus::Ok;
    }

    // Receives exactly `len` bytes into `buf`, stopping at the first error. The timeout
    // applies to each byte, not to the whole buffer.
    __attribute__((warn_unused_result))
    static UartStatus read(uint8_t* buf, size_t len, uint32_t timeoutMs) {
        for (size_t i = 0; i < len; ++i) {
            const UartStatus status = read(buf[i], timeoutMs);
            if (status != UartStatus::Ok) {
                return status;
            }
        }
        return UartStatus::Ok;
    }
};
} // namespace hal