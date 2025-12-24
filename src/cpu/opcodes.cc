#include "cpu.h"

#include <cstdlib>

void CPU::opcode_nop() {
    
}

void CPU::opcode_ld(ByteRegister& reg){
    u8 n = get_byte_from_pc();
    reg.set(n);
}