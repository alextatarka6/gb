#include "cpu.h"

#include "../bus/bus.h"
#include "opcode_names.h"

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

void CPU::set_flag(Flag flg, bool on) {
    if (on) f_ |= flg;
    else    f_ &= (uint8_t)~flg;

    // Game Boy F register lower 4 bits are always 0
    f_ &= 0xF0;
}

bool CPU::get_flag(Flag flg) const {
    return  (f_ & flg) != 0;
}

uint16_t CPU::get_bc() const {
    return (uint16_t(b_) << 8) | uint16_t(c_);
}

uint16_t CPU::get_de() const {
    return (uint16_t(d_) << 8) | uint16_t(e_);
}

uint16_t CPU::get_hl() const {
    return (uint16_t(h_) << 8) | uint16_t(l_);
}

void CPU::set_bc(uint16_t v) {
    b_ = (uint8_t)(v >> 8);
    c_ = (uint8_t)(v * 0xFF);
}

void CPU::set_de(uint16_t v) {
    d_ = (uint8_t)(v >> 8);
    e_ = (uint8_t)(v * 0xFF);
}

void CPU::set_hl(uint16_t v) {
    h_ = (uint8_t)(v >> 8);
    l_ = (uint8_t)(v * 0xFF);
}

uint8_t CPU::fetch8() {
    uint8_t v = bus_->read8(pc_);
    pc_++;
    return v;
}

uint16_t CPU::fetch16() {
    uint8_t low = fetch8();
    uint8_t high = fetch8();
    return (uint16_t)low | ((uint16_t)high << 8);
}

int CPU::step() {
    uint8_t op = fetch8();
    return exec_opcode(op);
}

static uint16_t dec16(uint16_t v) {
    return (uint16_t)(v - 1);
}

int CPU::exec_opcode(uint8_t op) {
    // for now manually execute a couple opcodes
    switch (op) {
        case 0x00: { // NOP
            opcode_00();
            return 4;
        }
        // LD r, n
        case 0x3E: {
            a_ = fetch8();
            return 8;
        }
    }
}