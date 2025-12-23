#pragma once

#include <cstdint>
#include <array>

class Cartridge;

class Bus {
public:
    explicit Bus(Cartridge* cart);

    uint8_t read8(uint16_t addr) const;
    void write8(uint16_t addr, uint8_t value);

    uint16_t read16(uint16_t addr) const;
    void write16(uint16_t addr, uint16_t value);

private:
    Cartridge* cart_ = nullptr;

    // DMG Sizes:
    // WRAM: 8 KiB at 0xC000..0xDFFF
    std::array<uint8_t, 0x2000> wram_{};

    // HRAM: 127 bytes at 0xFF80..0xFFEE
    std::array<uint8_t, 0x7F> hram_{};
};