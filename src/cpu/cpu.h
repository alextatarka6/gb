#pragma once

#include <cstdint>

class Bus;

class CPU {
public:
    explicit CPU(Bus* bus);

    void reset();               // set registers to power on state
    int step();                 // execute 1 instruction, return cycles

    uint16_t pc() const { return pc_; }

private:
    Bus* bus_ = nullptr;

    // 8-bit regs
    uint8_t a_ = 0, f_ = 0;
    uint8_t b_ = 0, c_ = 0;
    uint8_t d_ = 0, e_ = 0;
    uint8_t h_ = 0, l_ = 0;

    // 16-bit regs
    uint16_t sp_ = 0;
    uint16_t pc_ = 0;

    // flag helpers
    enum Flag : uint8_t {
        Z = 1 << 7,
        N = 1 << 6,
        H = 1 << 5,
        C = 1 << 4
    };

    void set_flag(Flag flg, bool on);
    bool get_flag(Flag flg) const;

    // register pair helpers
    uint16_t get_bc() const;
    uint16_t get_de() const;
    uint16_t get_hl() const;
    void set_bc(uint16_t v);
    void set_de(uint16_t v);
    void set_hl(uint16_t v);

    // fetch helpers
    uint8_t fetch8();
    uint16_t fetch16();

    // execute helpers for the first few instructions
    int exec_opcode(uint8_t op);

    // opcode helper functions
    void opcode_nop();

    // opcodes
    void opcode_00(); void opcode_01();
};