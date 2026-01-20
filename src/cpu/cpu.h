#pragma once

#include "../address.h"
#include "../register.h"
#include "../options.h"

class Gameboy;

// flag helpers
enum class Condition {
    NZ,
    Z,
    NC,
    C
};

namespace rst {
const u16 rst1 = 0x00;
const u16 rst2 = 0x08;
const u16 rst3 = 0x10;
const u16 rst4 = 0x18;
const u16 rst5 = 0x20;
const u16 rst6 = 0x28;
const u16 rst7 = 0x30;
const u16 rst8 = 0x38;
}; // namespace rst

namespace interrupts {
const u16 vblank = 0x40;
const u16 lcd_status = 0x48;
const u16 timer = 0x50;
const u16 serial = 0x58;
const u16 joypad = 0x60;
}; // namespace interrupts

class CPU {
public:
    CPU(Gameboy& inGb, Options& options);

    auto tick() -> Cycles;

    auto execute_opcode(u8 opcode, u16 opcode_pc) -> Cycles;

    auto execute_normal_opcode(u8 opcode, u16 opcode_pc) -> Cycles;
    auto execute_cb_opcode(u8 opcode, u16 opcode_pc) -> Cycles;

    ByteRegister interrupt_flag;
    ByteRegister interrupt_enabled;

private:
    void handle_interrupts();
    auto handle_interrupt(u8 interrupt_bit, u16 interrupt_vector, u8 fired_interrupts) -> bool;

    Gameboy& gb;
    Options& options;

    bool interrupts_enabled = false;
    bool halted = false;

    bool branch_taken = false;

    // 8-bit regs
    ByteRegister a, b, c, d, e, h, l;

    // Group Registers
    RegisterPair af;
    RegisterPair bc;
    RegisterPair de;
    RegisterPair hl;

    // Flags set dependent on the result of the last operation
    // 0x80 - produced 0
    // 0x40 - was a subtraction
    // 0x20 - lower half of byte overflowed 15
    // 0x10 - overflowed 255 or underflowed 0 for addition/subtraction
    FlagRegister f;

    void set_flag_zero(bool set);
    void set_flag_subtract(bool set);
    void set_flag_half_carry(bool set);
    void set_flag_carry(bool set);

    // Note that it's not const because this also sets the 'branch_taken' flag if a branch is taken
    // This allows the correct cycle count to be used
    auto is_condition(Condition condition) -> bool;

    // Program Counter
    WordRegister pc;

    // Stack Pointer
    WordRegister sp;

    auto get_byte_from_pc() -> u8;
    auto get_signed_byte_from_pc() -> s8;
    auto get_word_from_pc() -> u16;

    void stack_push(const WordValue& reg);
    void stack_pop(WordValue& reg);

    /* Opcode Helper Functions */

    // ADC
    void _opcode_adc(u8 value);

    void opcode_adc();
    void opcode_adc(const ByteRegister& reg);
    void opcode_adc(const Address&& addr);

    // ADD
    void _opcode_add(u8 reg, u8 value);

    void opcode_add_a();
    void opcode_add_a(const ByteRegister& reg);
    void opcode_add_a(const Address&& addr);

    void opcode_add_hl(const u16 value);
    void opcode_add_hl(const RegisterPair& reg_pair);
    void opcode_add_hl(const WordRegister& word_reg);

    void opcode_add_sp();

    // AND
    void _opcode_and(u8 value);

    void opcode_and();
    void opcode_and(const ByteRegister& reg);
    void opcode_and(const Address&& addr);

    // BIT
    void _opcode_bit(u8 bit, u8 value);

    void opcode_bit(u8 bit, ByteRegister& reg);
    void opcode_bit(u8 bit, const Address&& addr);

    // CALL
    void opcode_call();
    void opcode_call(Condition condition);

    // CCF
    void opcode_ccf();

    // CP
    void _opcode_cp(u8 value);

    void opcode_cp();
    void opcode_cp(const ByteRegister& reg);
    void opcode_cp(const Address&& addr);

    // CPL
    void opcode_cpl();

    // DAA
    void opcode_daa();

    // DEC
    void opcode_dec(ByteRegister& reg);
    void opcode_dec(RegisterPair& reg_pair);
    void opcode_dec(WordRegister& word_reg);
    void opcode_dec(Address&& addr);

    // DI
    void opcode_di();

    // EI
    void opcode_ei();

    // INC
    void opcode_inc(ByteRegister& reg);
    void opcode_inc(RegisterPair& reg_pair);
    void opcode_inc(WordRegister& word_reg);
    void opcode_inc(Address&& addr);

    // JP
    void opcode_jp();
    void opcode_jp(Condition condition);
    void opcode_jp(Address&& addr);

    // NOP
    void opcode_nop();

    // LD
    void opcode_ld(ByteRegister& reg);
    void opcode_ld(ByteRegister&, ByteRegister&);

    // opcodes
    void opcode_00(); void opcode_01(); void opcode_02(); void opcode_03(); void opcode_04(); void opcode_05(); void opcode_06(); void opcode_07(); void opcode_08(); void opcode_09(); void opcode_0A(); void opcode_0B(); void opcode_0C(); void opcode_0D(); void opcode_0E(); void opcode_0F();
    void opcode_10(); void opcode_11(); void opcode_12(); void opcode_13(); void opcode_14(); void opcode_15(); void opcode_16(); void opcode_17(); void opcode_18(); void opcode_19(); void opcode_1A(); void opcode_1B(); void opcode_1C(); void opcode_1D(); void opcode_1E(); void opcode_1F();
    void opcode_20(); void opcode_21(); void opcode_22(); void opcode_23(); void opcode_24(); void opcode_25(); void opcode_26(); void opcode_27(); void opcode_28(); void opcode_29(); void opcode_2A(); void opcode_2B(); void opcode_2C(); void opcode_2D(); void opcode_2E(); void opcode_2F();
    void opcode_30(); void opcode_31(); void opcode_32(); void opcode_33(); void opcode_34(); void opcode_35(); void opcode_36(); void opcode_37(); void opcode_38(); void opcode_39(); void opcode_3A(); void opcode_3B(); void opcode_3C(); void opcode_3D(); void opcode_3E(); void opcode_3F();
    void opcode_40(); void opcode_41(); void opcode_42(); void opcode_43(); void opcode_44(); void opcode_45(); void opcode_46(); void opcode_47(); void opcode_48(); void opcode_49(); void opcode_4A(); void opcode_4B(); void opcode_4C(); void opcode_4D(); void opcode_4E(); void opcode_4F();
    void opcode_50(); void opcode_51(); void opcode_52(); void opcode_53(); void opcode_54(); void opcode_55(); void opcode_56(); void opcode_57(); void opcode_58(); void opcode_59(); void opcode_5A(); void opcode_5B(); void opcode_5C(); void opcode_5D(); void opcode_5E(); void opcode_5F();
    void opcode_60(); void opcode_61(); void opcode_62(); void opcode_63(); void opcode_64(); void opcode_65(); void opcode_66(); void opcode_67(); void opcode_68(); void opcode_69(); void opcode_6A(); void opcode_6B(); void opcode_6C(); void opcode_6D(); void opcode_6E(); void opcode_6F();
    void opcode_70(); void opcode_71(); void opcode_72(); void opcode_73(); void opcode_74(); void opcode_75(); void opcode_76(); void opcode_77(); void opcode_78(); void opcode_79(); void opcode_7A(); void opcode_7B(); void opcode_7C(); void opcode_7D(); void opcode_7E(); void opcode_7F();
    void opcode_80(); void opcode_81(); void opcode_82(); void opcode_83(); void opcode_84(); void opcode_85(); void opcode_86(); void opcode_87(); void opcode_88(); void opcode_89(); void opcode_8A(); void opcode_8B(); void opcode_8C(); void opcode_8D(); void opcode_8E(); void opcode_8F();
    void opcode_90(); void opcode_91(); void opcode_92(); void opcode_93(); void opcode_94(); void opcode_95(); void opcode_96(); void opcode_97(); void opcode_98(); void opcode_99(); void opcode_9A(); void opcode_9B(); void opcode_9C(); void opcode_9D(); void opcode_9E(); void opcode_9F();
    void opcode_A0(); void opcode_A1(); void opcode_A2(); void opcode_A3(); void opcode_A4(); void opcode_A5(); void opcode_A6(); void opcode_A7(); void opcode_A8(); void opcode_A9(); void opcode_AA(); void opcode_AB(); void opcode_AC(); void opcode_AD(); void opcode_AE(); void opcode_AF();
    void opcode_B0(); void opcode_B1(); void opcode_B2(); void opcode_B3(); void opcode_B4(); void opcode_B5(); void opcode_B6(); void opcode_B7(); void opcode_B8(); void opcode_B9(); void opcode_BA(); void opcode_BB(); void opcode_BC(); void opcode_BD(); void opcode_BE(); void opcode_BF();
    void opcode_C0(); void opcode_C1(); void opcode_C2(); void opcode_C3(); void opcode_C4(); void opcode_C5(); void opcode_C6(); void opcode_C7(); void opcode_C8(); void opcode_C9(); void opcode_CA(); void opcode_CB(); void opcode_CC(); void opcode_CD(); void opcode_CE(); void opcode_CF();
    void opcode_D0(); void opcode_D1(); void opcode_D2(); void opcode_D3(); void opcode_D4(); void opcode_D5(); void opcode_D6(); void opcode_D7(); void opcode_D8(); void opcode_D9(); void opcode_DA(); void opcode_DB(); void opcode_DC(); void opcode_DD(); void opcode_DE(); void opcode_DF();
    void opcode_E0(); void opcode_E1(); void opcode_E2(); void opcode_E3(); void opcode_E4(); void opcode_E5(); void opcode_E6(); void opcode_E7(); void opcode_E8(); void opcode_E9(); void opcode_EA(); void opcode_EB(); void opcode_EC(); void opcode_ED(); void opcode_EE(); void opcode_EF();
    void opcode_F0(); void opcode_F1(); void opcode_F2(); void opcode_F3(); void opcode_F4(); void opcode_F5(); void opcode_F6(); void opcode_F7(); void opcode_F8(); void opcode_F9(); void opcode_FA(); void opcode_FB(); void opcode_FC(); void opcode_FD(); void opcode_FE(); void opcode_FF();
};