#include "cpu.h"
#include "../bus/bus.h"

CPU::CPU(Bus* bus) : bus_(bus) {}

void CPU::reset() {
    // for now simply reset registers
    a_ = 0;
    f_ = 0;
    b_ = 0;
    c_ = 0;
    d_ = 0;
    e_ = 0;
    h_ = 0;
    l_ = 0;

    pc_ = 0x0100;
    sp_ = 0xFFFE;
}