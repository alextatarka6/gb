#include "bus.h"
#include "../cart/cartridge.h"

Bus::Bus(Cartridge* cart) : cart_(cart) {}

uint8_t Bus::read8(uint16_t addr) const {
    // ROM 0x0000..0x7FFF
    if (addr <= 0x7FFF) {
        return cart_ ? cart_->read8(addr) : 0xFF;
    }

    // WRAM 0xC000..0xDFFF
    if (addr >= 0xC000 && addr <= 0xDFFF) {
        return wram_[addr - 0xC000];
    }

    // HRAM 0xFF80..0xFFEE
    if (addr >= 0xFF80 && addr <= 0xFFEE) {
        return hram_[addr - 0xFF80];
    }

    return 0xFF;
}

void Bus::write8(uint16_t addr, uint8_t value) {
    // ROM region writes will later go to MBC. For now ignore.
    if (addr <= 0x7FFF) {
        return;
    }

    // WRAM
    if (addr >= 0xC000 && addr <= 0xDFFF) {
        wram_[addr - 0xC000] = value;
        return;
    }

    // HRAM
    if (addr >= 0xFF80 && addr <= 0xFFEE) {
        hram_[addr - 0xFF80] = value;
        return;
    }

    // ignore everything else for now
}

// Game-Boy is little-endian: low byte addr, high byte at addr + 1

uint16_t Bus::read16(uint16_t addr) const {
    uint8_t low = read8(addr);
    uint8_t high = read8(addr + 1);
    return (uint16_t)low | ((uint16_t)high << 8);
}

void Bus::write16(uint16_t addr, uint16_t value) {
    write8(addr, (uint8_t)(value & 0xFF));
    write8(addr + 1, (uint8_t)(value >> 8));
}