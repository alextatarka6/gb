#include "cpu.h"

#include "../gameboy.h"
#include "../util/bitwise.h"

#include <cstdlib>

using bitwise::check_bit;
using bitwise::clear_bit;
using bitwise::set_bit;

// ADC
void CPU::_opcode_adc(u8 value) {
    u8 reg = a.value();
    u8 carry = f.flag_carry_value();

    uint result_full = reg + value + carry;
    u8 result = static_cast<u8>(result_full);
    
    set_flag_zero(result == 0);
    set_flag_subtract(false);
    set_flag_half_carry((reg & 0xF) + (value & 0xF) > 0xF);
    set_flag_carry(result_full > 0xFF);

    a.set(result);
}

void CPU::opcode_adc() {
    _opcode_adc(get_byte_from_pc());
}

void CPU::opcode_adc(const ByteRegister& reg) {
    _opcode_adc(reg.value());
}

void CPU::opcode_adc(const Address&& addr) {
    _opcode_adc(gb.mmu.read(addr));
}

// ADD
void CPU::_opcode_add(u8 reg, u8 value) {
    uint result_full = reg + value;
    a.set(static_cast<u8>(result_full));

    set_flag_zero(a.value() == 0);
    set_flag_subtract(false);
    set_flag_half_carry((reg & 0xF) + (value & 0xF) > 0xF);
    set_flag_carry((result_full & 0x100) != 0);
}

void CPU::opcode_add_a() {
    _opcode_add(a.value(), get_byte_from_pc());
}

void CPU::opcode_add_a(const ByteRegister& reg) {
    _opcode_add(a.value(), reg.value());
}

void CPU::opcode_add_a(const Address&& addr) {
    _opcode_add(a.value(), gb.mmu.read(addr));
}

void CPU::opcode_add_hl(const u16 value) {
    u16 reg = hl.value();
    uint result_full = reg + value;

    set_flag_subtract(false);
    set_flag_half_carry((reg & 0xFFF) + (value & 0xFFF) > 0xFFF);
    set_flag_carry((result_full & 0x10000) != 0);

    hl.set(static_cast<u16>(result_full));
}

void CPU::opcode_add_hl(const RegisterPair& reg_pair) {
    opcode_add_hl(reg_pair.value());
}

void CPU::opcode_add_hl(const WordRegister& word_reg) {
    opcode_add_hl(word_reg.value());
}

void CPU::opcode_add_sp() {
    s8 value = get_signed_byte_from_pc();
    u16 reg = sp.value();
    uint result_full = reg + value;

    set_flag_zero(false);
    set_flag_subtract(false);
    set_flag_half_carry(((reg ^ value ^ (result_full & 0xFFFF)) & 0x10) == 0x10);
    set_flag_carry(((reg ^ value ^ (result_full & 0xFFFF)) & 0x100) == 0x100);

    sp.set(static_cast<u16>(result_full));
}

// AND
void CPU::_opcode_and(u8 value) {
    u8 reg = a.value();
    u8 result = reg & value;

    a.set(result);

    set_flag_zero(result == 0);
    set_flag_subtract(false);
    set_flag_half_carry(true);
    set_flag_carry(false);
}

void CPU::opcode_and() {
    _opcode_and(get_byte_from_pc());
}

void CPU::opcode_and(const ByteRegister& reg) {
    _opcode_and(reg.value());
}

void CPU::opcode_and(const Address&& addr) {
    _opcode_and(gb.mmu.read(addr));
}

// BIT
void CPU::_opcode_bit(u8 bit, u8 value) {
    bool bit_set = check_bit(value, bit);

    set_flag_zero(!bit_set);
    set_flag_subtract(false);
    set_flag_half_carry(true);
}

void CPU::opcode_bit(u8 bit, ByteRegister& reg) {
    _opcode_bit(bit, reg.value());
}

void CPU::opcode_bit(u8 bit, const Address&& addr) {
    _opcode_bit(bit, gb.mmu.read(addr));
}

// CALL
void CPU::opcode_call() {
    u16 address = get_word_from_pc();

    stack_push(pc);
    pc.set(address);
}

void CPU::opcode_call(Condition condition) {
    if (is_condition(condition)) {
        opcode_call();
    }
}

// CCF
void CPU::opcode_ccf() {
    set_flag_subtract(false);
    set_flag_half_carry(false);
    set_flag_carry(!f.flag_carry());
}

// CP
void CPU::_opcode_cp(u8 value) {
    u8 reg = a.value();
    u8 result = static_cast<u8>(reg - value);

    set_flag_zero(result == 0);
    set_flag_subtract(true);
    set_flag_half_carry((reg & 0xF) < (value & 0xF));
    set_flag_carry(reg < value);
}

void CPU::opcode_cp() {
    _opcode_cp(get_byte_from_pc());
}

void CPU::opcode_cp(const ByteRegister& reg) {
    _opcode_cp(reg.value());
}

void CPU::opcode_cp(const Address&& addr) {
    _opcode_cp(gb.mmu.read(addr));
}

// CPL
void CPU::opcode_cpl() {
    a.set(~a.value());

    set_flag_subtract(true);
    set_flag_half_carry(true);
}

// DAA
void CPU::opcode_daa() {
    u8 reg = a.value();

    u16 correction = f.flag_carry() ? 0x60 : 0x00;

    if (f.flag_half_carry() || (!f.flag_subtract() && (reg & 0x0F) > 0x09)) {
        correction |= 0x06;
    }

    if (f.flag_carry() || (!f.flag_subtract() && reg > 0x99)) {
        correction |= 0x60;
    }

    if (f.flag_subtract()) {
        reg = static_cast<u8>(reg - correction);
    } else {
        reg = static_cast<u8>(reg + correction);
    }

    if (((correction << 2) & 0x100) != 0) {
        set_flag_carry(true);
    }

    set_flag_half_carry(false);
    set_flag_zero(reg == 0);

    a.set(static_cast<u8>(reg));
}

// DEC
void CPU::opcode_dec(ByteRegister& reg) {
    reg.decrement();

    set_flag_zero(reg.value() == 0);
    set_flag_subtract(true);
    set_flag_half_carry((reg.value() & 0x0F) == 0x0F);
}

void CPU::opcode_dec(RegisterPair& reg_pair) {
    reg_pair.decrement();
}

void CPU::opcode_dec(WordRegister& word_reg) {
    word_reg.decrement();
}

void CPU::opcode_dec(Address&& addr) {
    u8 value = gb.mmu.read(addr);
    value = static_cast<u8>(value - 1);
    gb.mmu.write(addr, value);

    set_flag_zero(value == 0);
    set_flag_subtract(true);
    set_flag_half_carry((value & 0x0F) == 0x0F);
}

// DI
void CPU::opcode_di() {
    interrupts_enabled = false;
}

// EI
void CPU::opcode_ei() {
    interrupts_enabled = true;
}

// INC
void CPU::opcode_inc(ByteRegister& reg) {
    reg.increment();

    set_flag_zero(reg.value() == 0);
    set_flag_subtract(false);
    set_flag_half_carry((reg.value() & 0x0F) == 0x0F);
}

void CPU::opcode_inc(RegisterPair& reg_pair) {
    reg_pair.increment();
}

void CPU::opcode_inc(WordRegister& word_reg) {
    word_reg.increment();
}

void CPU::opcode_inc(Address&& addr) {
    u8 value = gb.mmu.read(addr);
    value = static_cast<u8>(value + 1);
    gb.mmu.write(addr, value);

    set_flag_zero(value == 0);
    set_flag_subtract(false);
    set_flag_half_carry((value & 0x0F) == 0x0F);
}

// JP
void CPU::opcode_jp() {
    u16 address = get_word_from_pc();
    pc.set(address);
}

void CPU::opcode_jp(Condition condition) {
    if (is_condition(condition)) {
        opcode_jp();
    }
    else {
        get_word_from_pc();  // consume unused address
    }
}

void CPU::opcode_jp(Address&& addr) {
    unused(addr);
    pc.set(hl.value());
}

// NOP
void CPU::opcode_nop() {
    
}

// LD
void CPU::opcode_ld(ByteRegister& reg){
    u8 n = get_byte_from_pc();
    reg.set(n);
}

void CPU::opcode_ld(ByteRegister& reg, ByteRegister& other){
    reg.set(other.value());
}