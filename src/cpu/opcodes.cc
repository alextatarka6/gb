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

// JR
void CPU::opcode_jr() {
    s8 offset = get_signed_byte_from_pc();
    u16 new_pc = static_cast<u16>(pc.value() + offset);
    pc.set(new_pc);
}

void CPU::opcode_jr(Condition condition) {
    if (is_condition(condition)) {
        opcode_jr();
    }
    else {
        get_byte_from_pc(); // consume unused offset
    }
}

// HALT
void CPU::opcode_halt() {
    halted = true;
}

// LD
void CPU::opcode_ld(ByteRegister& reg){
    u8 n = get_byte_from_pc();
    reg.set(n);
}

void CPU::opcode_ld(ByteRegister& reg, const ByteRegister& byte_reg){
    reg.set(byte_reg.value());
}

void CPU::opcode_ld(ByteRegister& reg, const Address&& addr){
    reg.set(gb.mmu.read(addr));
}

void CPU::opcode_ld_from_addr(ByteRegister& reg) {
    u16 nn = get_word_from_pc();
    reg.set(gb.mmu.read(nn));
}

void CPU::opcode_ld(RegisterPair& reg_pair) {
    u16 nn = get_word_from_pc();
    reg_pair.set(nn);
}

void CPU::opcode_ld(WordRegister& word_reg) {
    u16 nn = get_word_from_pc();
    word_reg.set(nn);
}

void CPU::opcode_ld(WordRegister& word_reg, const RegisterPair& reg_pair) {
    word_reg.set(reg_pair.value());
}

void CPU::opcode_ld(const Address& addr) {
    u8 n = get_byte_from_pc();
    gb.mmu.write(addr, n);
}

void CPU::opcode_ld(const Address& addr, const ByteRegister& reg) {
    gb.mmu.write(addr, reg.value());
}

void CPU::opcode_ld(const Address& addr, const WordRegister& word_reg) {
    gb.mmu.write(addr, word_reg.low());
    gb.mmu.write(addr + 1, word_reg.high());
}

void CPU::opcode_ld_to_addr(const ByteRegister& reg) {
    u16 nn = get_word_from_pc();
    gb.mmu.write(nn, reg.value());
}

// LDD
void CPU::opcode_ldd(ByteRegister& reg, const Address& addr) {
    reg.set(gb.mmu.read(addr));
    hl.decrement();
}

void CPU::opcode_ldd(const Address& addr, const ByteRegister& reg) {
    gb.mmu.write(addr, reg.value());
    hl.decrement();
}

// LDH
void CPU::opcode_ldh_into_a() {
    u8 offset = get_byte_from_pc();
    auto address = Address(0xFF00 + offset);
    a.set(gb.mmu.read(address));
}

void CPU::opcode_ldh_into_data() {
    u8 offset = get_byte_from_pc();
    auto address = Address(0xFF00 + offset);
    gb.mmu.write(address, a.value());
}

void CPU::opcode_ldh_into_c() {
    u8 offset = c.value();
    auto address = Address(0xFF00 + offset);
    gb.mmu.write(address, a.value());
}

void CPU::opcode_ldh_c_into_a() {
    u8 offset = c.value();
    auto address = Address(0xFF00 + offset);
    a.set(gb.mmu.read(address));
}

// LDHL
void CPU::opcode_ldhl() {
    u16 reg = sp.value();
    s8 n = get_signed_byte_from_pc();
    int result = static_cast<int>(sp.value() + n);

    set_flag_zero(false);
    set_flag_subtract(false);
    set_flag_half_carry(((reg ^ n ^ (result & 0xFFFF)) & 0x10) == 0x10);
    set_flag_carry(((reg ^ n ^ (result & 0xFFFF)) & 0x100) == 0x100);

    hl.set(static_cast<u16>(result));
}

// LDI
void CPU::opcode_ldi(ByteRegister& reg, const Address& addr) {
    reg.set(gb.mmu.read(addr));
    hl.increment();
}

void CPU::opcode_ldi(const Address& addr, const ByteRegister& reg) {
    gb.mmu.write(addr, reg.value());
    hl.increment();
}

// NOP
void CPU::opcode_nop() {
    
}

// OR
void CPU::_opcode_or(u8 value) {
    u8 reg = a.value();
    u8 result = reg | value;

    a.set(result);

    set_flag_zero(result == 0);
}

void CPU::opcode_or() {
    _opcode_or(get_byte_from_pc());
}

void CPU::opcode_or(const ByteRegister& reg) {
    _opcode_or(reg.value());
}

void CPU::opcode_or(const Address&& addr) {
    _opcode_or(gb.mmu.read(addr));
}

// POP
void CPU::opcode_pop(RegisterPair& reg_pair) {
    stack_pop(reg_pair);
}

// PUSH
void CPU::opcode_push(const RegisterPair& reg_pair) {
    stack_push(reg_pair);
}

// RES
void CPU::opcode_res(u8 bit, ByteRegister& reg) {
    u8 result = clear_bit(reg.value(), bit);
    reg.set(result);
}

void CPU::opcode_res(u8 bit, Address&& addr) {
    u8 value = gb.mmu.read(addr);
    u8 result = clear_bit(value, bit);
    gb.mmu.write(addr, result);
}

// RET
void CPU::opcode_ret() {
    stack_pop(pc);
}

void CPU::opcode_ret(Condition condition) {
    if (is_condition(condition)) {
        opcode_ret();
    }
}

// RETI
void CPU::opcode_reti() {
    opcode_ret();
    interrupts_enabled = true;
}

// RL
auto CPU::_opcode_rl(u8 value) -> u8 {
    u8 carry = f.flag_carry_value();
    bool will_carry = check_bit(value, 7);

    u8 result = static_cast<u8>(value << 1);
    result |= carry;

    set_flag_zero(result == 0);
    set_flag_subtract(false);
    set_flag_half_carry(false);
    set_flag_carry(will_carry);

    return result;
}

void CPU::opcode_rla() {
    opcode_rl(a);
    set_flag_zero(false);
}

void CPU::opcode_rl(ByteRegister& reg) {
    reg.set(_opcode_rl(reg.value()));
}

void CPU::opcode_rl(Address&& addr) {
    u8 value = gb.mmu.read(addr);
    u8 result = _opcode_rl(value);
    gb.mmu.write(addr, result);
}

// RLC
auto CPU::_opcode_rlc(u8 value) -> u8 {
    u8 carry_flag = check_bit(value, 7);
    u8 truncated_bit = check_bit(value, 7);
    u8 result = static_cast<u8>((value << 1) | truncated_bit);

    set_flag_carry(carry_flag);
    set_flag_zero(result == 0);
    set_flag_half_carry(false);
    set_flag_subtract(false);

    return result;
}

void CPU::opcode_rlca() {
    opcode_rlc(a);
    set_flag_zero(false);
}

void CPU::opcode_rlc(ByteRegister& reg) {
    reg.set(_opcode_rlc(reg.value()));
}

void CPU::opcode_rlc(Address&& addr) {
    u8 value = gb.mmu.read(addr);
    u8 result = _opcode_rlc(value);
    gb.mmu.write(addr, result);
}

// RR
auto CPU::_opcode_rr(u8 value) -> u8 {
    u8 carry = f.flag_carry_value();

    bool will_carry = check_bit(value, 0);
    set_flag_carry(will_carry);

    u8 result = static_cast<u8>(value >> 1);
    result |= static_cast<u8>(carry << 7);

    set_flag_zero(result == 0);
    set_flag_subtract(false);
    set_flag_half_carry(false);
}

void CPU::opcode_rra() {
    opcode_rr(a);
    set_flag_zero(false);
}

void CPU::opcode_rr(ByteRegister& reg) {
    reg.set(_opcode_rr(reg.value()));
}

void CPU::opcode_rr(Address&& addr) {
    u8 result = _opcode_rr(gb.mmu.read(addr));
    gb.mmu.write(addr, result);
}

// RRC
auto CPU::_opcode_rrc(u8 value) -> u8 {
    u8 carry_flag = check_bit(value, 0);
    u8 truncated_bit = check_bit(value, 0);
    u8 result = static_cast<u8>((value >> 1) | (truncated_bit << 7));

    set_flag_carry(carry_flag);
    set_flag_zero(result == 0);
    set_flag_half_carry(false);
    set_flag_subtract(false);

    return result;
}

void CPU::opcode_rrca() {
    opcode_rrc(a);
    set_flag_zero(false);
}

void CPU::opcode_rrc(ByteRegister& reg) {
    reg.set(_opcode_rrc(reg.value()));
}

void CPU::opcode_rrc(Address&& addr) {
    u8 result = _opcode_rrc(gb.mmu.read(addr));
    gb.mmu.write(addr, result);
}

// RST
void CPU::opcode_rst(const u8 offset) {
    stack_push(pc);
    pc.set(offset);
}

// SBC
void CPU::_opcode_sbc(u8 value) {
    u8 reg = a.value();
    u8 carry = f.flag_carry_value();

    uint result_full = reg - value - carry;
    u8 result = static_cast<u8>(result_full);

    set_flag_zero(result == 0);
    set_flag_subtract(true);
    set_flag_half_carry((reg & 0xF) < ((value & 0xF) + carry));
    set_flag_carry(reg < (value + carry));

    a.set(result);
}

void CPU::opcode_sbc() {
    _opcode_sbc(get_byte_from_pc());
}

void CPU::opcode_sbc(const ByteRegister& reg) {
    _opcode_sbc(reg.value());
}

void CPU::opcode_sbc(const Address&& addr) {
    _opcode_sbc(gb.mmu.read(addr));
}

// SCF
void CPU::opcode_scf() {
    set_flag_subtract(false);
    set_flag_half_carry(false);
    set_flag_carry(true);
}

// SET
void CPU::opcode_set(u8 bit, ByteRegister& reg) {
    u8 result = set_bit(reg.value(), bit);
    reg.set(result);
}

void CPU::opcode_set(u8 bit, Address&& addr) {
    u8 value = gb.mmu.read(addr);
    u8 result = set_bit(value, bit);
    gb.mmu.write(addr, result);
}

// SLA
auto CPU::_opcode_sla(u8 value) -> u8 {
    bool will_carry = check_bit(value, 7);
    u8 result = static_cast<u8>(value << 1);
    set_flag_zero(result == 0);
    set_flag_subtract(false);
    set_flag_half_carry(false);
    set_flag_carry(will_carry);
    return result;
}

void CPU::opcode_sla(ByteRegister& reg) {
    reg.set(_opcode_sla(reg.value()));
}

void CPU::opcode_sla(Address&& addr) {
    u8 value = gb.mmu.read(addr);
    u8 result = _opcode_sla(value);
    gb.mmu.write(addr, result);
}

// SRA
auto CPU::_opcode_sra(u8 value) -> u8 {
    bool will_carry = check_bit(value, 0);
    u8 top_bit = check_bit(value, 7) ? 0x80 : 0x00;
    u8 result = static_cast<u8>((value >> 1) | top_bit);
    set_flag_zero(result == 0);
    set_flag_subtract(false);
    set_flag_half_carry(false);
    set_flag_carry(will_carry);
    return result;
}

void CPU::opcode_sra(ByteRegister& reg) {
    reg.set(_opcode_sra(reg.value()));
}

void CPU::opcode_sra(Address&& addr) {
    u8 value = gb.mmu.read(addr);
    u8 result = _opcode_sra(value);
    gb.mmu.write(addr, result);
}

// SRL
auto CPU::_opcode_srl(u8 value) -> u8 {
    bool will_carry = check_bit(value, 0);
    u8 result = static_cast<u8>(value >> 1);
    set_flag_zero(result == 0);
    set_flag_subtract(false);
    set_flag_half_carry(false);
    set_flag_carry(will_carry);
    return result;
}

void CPU::opcode_srl(ByteRegister& reg) {
    reg.set(_opcode_srl(reg.value()));
}

void CPU::opcode_srl(Address&& addr) {
    u8 value = gb.mmu.read(addr);
    u8 result = _opcode_srl(value);
    gb.mmu.write(addr, result);
}

// SUB
void CPU::_opcode_sub(u8 value) {
    u8 reg = a.value();
    u8 result = static_cast<u8>(reg - value);

    a.set(result);

    set_flag_zero(result == 0);
    set_flag_subtract(true);
    set_flag_half_carry((reg & 0xF) < (value & 0xF));
    set_flag_carry(reg < value);
}

void CPU::opcode_sub() {
    _opcode_sub(get_byte_from_pc());
}

void CPU::opcode_sub(const ByteRegister& reg) {
    _opcode_sub(reg.value());
}

void CPU::opcode_sub(const Address&& addr) {
    _opcode_sub(gb.mmu.read(addr));
}

// SWAP

auto CPU::_opcode_swap(u8 value) -> u8 {
    u8 upper_nibble = (value & 0xF0) >> 4;
    u8 lower_nibble = (value & 0x0F) << 4;
    u8 result = upper_nibble | lower_nibble;

    set_flag_zero(result == 0);
    set_flag_subtract(false);
    set_flag_half_carry(false);
    set_flag_carry(false);

    return result;
}

void CPU::opcode_swap(ByteRegister& reg) {
    reg.set(_opcode_swap(reg.value()));
}

void CPU::opcode_swap(Address&& addr) {
    u8 value = gb.mmu.read(addr);
    u8 result = _opcode_swap(value);
    gb.mmu.write(addr, result);
}

// XOR
void CPU::_opcode_xor(u8 value) {
    u8 reg = a.value();
    u8 result = reg ^ value;

    a.set(result);

    set_flag_zero(result == 0);
    set_flag_subtract(false);
    set_flag_half_carry(false);
    set_flag_carry(false);
}

void CPU::opcode_xor() {
    _opcode_xor(get_byte_from_pc());
}

void CPU::opcode_xor(const ByteRegister& reg) {
    _opcode_xor(reg.value());
}

void CPU::opcode_xor(const Address&& addr) {
    _opcode_xor(gb.mmu.read(addr));
}

