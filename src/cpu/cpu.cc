#include "cpu.h"

#include "../bus/bus.h"
#include "opcode_names.h"
#include "../util/bitwise.h"

#include <iostream>>

using bitwise::compose_bytes;

CPU::CPU(Bus* bus) : 
    bus_(bus),
    af(a, f),
    bc(b, c),
    de(d, e),
    hl(h, l)
{
}

int CPU::tick() {
    u16 opcode_pc = pc.value();
    auto opcode = get_byte_from_pc();
    auto cycles = execute_opcode(opcode, opcode_pc);
    return cycles;
}

void CPU::reset() {
    // for now simply reset registers
    a.reset();
    f.reset();
    b.reset();
    c.reset();
    d.reset();
    e.reset();
    h.reset();
    l.reset();


    pc.set(0x0100);
    sp.set(0xFFFE);
}

void CPU::set_flag_zero(bool set) { f.set_flag_zero(set); }
void CPU::set_flag_subtract(bool set) { f.set_flag_subtract(set); }
void CPU::set_flag_half_carry(bool set) { f.set_flag_half_carry(set); }
void CPU::set_flag_carry(bool set) { f.set_flag_carry(set); }

auto CPU::get_byte_from_pc() -> u8 {
    u8 byte = bus_->read8(pc.value());
    pc.increment();

    return byte;
}

auto CPU::get_signed_byte_from_pc() -> s8 {
    s8 byte = bus_->read8(pc.value());
    pc.increment();

    return static_cast<s8>(byte);
}

auto CPU::get_word_from_pc() -> u16 {
    u8 low = get_byte_from_pc();
    u8 high = get_byte_from_pc();
    return compose_bytes(high, low);
}

auto CPU::execute_opcode(u8 opcode, u16 opcode_pc) ->  int {
    if (opcode == 0xCB) {
        u8 cb_opcode = get_byte_from_pc();
        return execute_cb_opcode(cb_opcode, opcode_pc);
    }
    return execute_normal_opcode(opcode, opcode_pc);
}

auto CPU::execute_normal_opcode(u8 opcode, u16 opcode_pc) -> int {
    // for now manually execute a couple opcodes
    switch (opcode) {
        case 0x00: { // NOP
            opcode_00();
            return 4;
        }
        // LD r, n
        case 0x3E: {
            opcode_ld(a);
            return 8;
        }
        case 0x06: {
            opcode_ld(b);
            return 8;
        }
        case 0x0E: {
            opcode_ld(c);
            return 8;
        }
        case 0x16: {
            opcode_ld(d);
            return 8;
        }
        case 0x1E: {
            opcode_ld(e);
            return 8;
        }
        case 0x26: {
            opcode_ld(h);
            return 8;
        }
        case 0x2E: {
            opcode_ld(l);
            return 8;
        }
        default: {
            // For learning: fail loudly so you know what opcode you hit.
            // You can replace this with a debug log and treat as NOP later.
            std::cerr << "Unimplemented opcode 0x"
                      << std::hex << (int)opcode
                      << " at PC=0x" << (int)(pc.value() - 1)
                      << std::dec << "\n";
            return 4; // pretend NOP so the program doesn't instantly lock, but you will see the log
        }
    }
}