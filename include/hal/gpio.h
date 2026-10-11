#pragma once

#include <sam.h>
#include <cstdint>

namespace hal {

enum class PeriphFunction: uint8_t { A, B, C, D, E, F, G, H };
enum class Pull: uint8_t { None, Up, Down };

template <uint8_t PortNum, uint8_t PinNum>
class OutputPin {
    static_assert(PortNum < 2, "SAMD21G has ports A (0) and B (1)");
    static_assert(PinNum < 32, "Pin number out of range");

    static constexpr uint32_t kMask = 1ul << PinNum;

public:
    static void init()   { PORT->Group[PortNum].DIRSET.reg = kMask; }
    static void set()    { PORT->Group[PortNum].OUTSET.reg = kMask; }
    static void clear()  { PORT->Group[PortNum].OUTCLR.reg = kMask; }
    static void toggle() { PORT->Group[PortNum].OUTTGL.reg = kMask; }
    static bool read()   { return (PORT->Group[PortNum].OUT.reg & kMask); }
};

template <uint8_t PortNum, uint8_t PinNum>
class InputPin {
    static_assert(PortNum < 2, "SAMD21G has ports A (0) and B (1)");
    static_assert(PinNum < 32, "Pin number out of range");

    static constexpr uint32_t kMask = 1ul << PinNum;

public:
    static void init(Pull pull) {
        PORT->Group[PortNum].DIRCLR.reg = kMask;
        uint8_t pinCfg = PORT_PINCFG_INEN | ((pull != Pull::None)? PORT_PINCFG_PULLEN: 0);
        if (pull == Pull::Up) {
            PORT->Group[PortNum].OUTSET.reg = kMask;
        }
        else if (pull == Pull::Down) {
            PORT->Group[PortNum].OUTCLR.reg = kMask;
        }
        PORT->Group[PortNum].PINCFG[PinNum].reg = pinCfg;
    }
    static bool read() { return (PORT->Group[PortNum].IN.reg & kMask); }
};

template<uint8_t PortNum, uint8_t PinNum>
class PeripheralPin {
    static_assert(PortNum < 2, "SAMD21G has ports A (0) and B (1)");
    static_assert(PinNum < 32, "Pin number out of range");

public:
    PeripheralPin() = delete;
    static void init(PeriphFunction fn) {
        if (PinNum % 2) {
            PORT->Group[PortNum].PMUX[PinNum >> 1].bit.PMUXO = static_cast<uint8_t>(fn);
        }
        else {
            PORT->Group[PortNum].PMUX[PinNum >> 1].bit.PMUXE = static_cast<uint8_t>(fn);
        }
        PORT->Group[PortNum].PINCFG[PinNum].reg = PORT_PINCFG_PMUXEN;
    }
};

template<typename Pin, bool ActiveHigh=true>
class Led {
public:
    Led() = delete;

    static void init() {
        Pin::init();
        off();
    }
    static void on() {
        if (ActiveHigh) {
            Pin::set();
        } else {
            Pin::clear();
        }
    }
    static void off() {
        if (ActiveHigh) {
            Pin::clear();
        } else {
            Pin::set();
        }
    }
    static void toggle() {
        Pin::toggle();
    }
    static bool isOn() {
        return Pin::read() == ActiveHigh;
    }
};

} // Namespace hal