#include "cpu.h"

#include "../util/bitwise.h"

#include <cstdlib>

using bitwise::check_bit;
using bitwise::clear_bit;
using bitwise::set_bit;

void CPU::opcode_nop() {
    
}

void CPU::opcode_ld(ByteRegister& reg){
    u8 n = get_byte_from_pc();
    reg.set(n);
}