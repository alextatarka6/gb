#include "mmu.h"
#include "gameboy.h"
#include "util/log.h"
#include "util/bitwise.h"
#include "cpu/cpu.h"

MMU::MMU(Gameboy& inGb, Options& inOptions) :
    gb(inGb),
    options(inOptions)
{
    work_ram = std::vector<u8>(0x8000);
    oam_ram = std::vector<u8>(0xA0);
    high_ram = std::vector<u8>(0x80);
}

auto MMU::read(const Address& address) const -> u8 {
    if (address.in_range(0x0, 0x7FFF)) {
        if (address.in_range(0x0, 0xFF) && boot_rom_active()) {
            // TODO: implement boot.h
            return;
        }
        return gb.cartridge->read(address);
    }

    // VRAM
    if (address.in_range(0x8000, 0x9FFF)) {
        // TODO: implement video read
        return 0xFF;
    }

    // External (cartridge) RAM
    if (address.in_range(0xA000, 0xBFFF)) {
        return gb.cartridge->read(address);
    }

    // Internal work RAM
    if (address.in_range(0xC000, 0xDFFF)) {
        return work_ram.at(address.value() - 0xC000);
    }

    if (address.in_range(0xE000, 0xFDFF)) {
        return read(address.value() - 0x2000);
    }

    // OAM
    if (address.in_range(0xFE00, 0xFE9F)) {
        return oam_ram.at(address.value() - 0xFE00);
    }

    if (address.in_range(0xFEA0, 0xFEFF)) {
        log_warn("Attempting to read from unsuable memory 0x%x", address.value());
        return 0xFF;
    }

    // Mapped IO
    if (address.in_range(0xFF00, 0xFF7F)) {
        return read_io(address);
    }

    // Zero Page ram
    if (address.in_range(0xFF80, 0xFFFE)) {
        return high_ram.at(address.value() - 0xFF80);
    }

    // Interrupt Enable register
    if (address == 0xFFFF) {
        // TODO: implement cpu interrupt enable register
        return 0xFF;
    }

    fatal_error("Attempted to read from unmapped memory address 0x%X", address.value());
}

auto MMU::read_io(const Address& address) const -> u8 {
    // TODO: implement hardware registers
    return 0xFF;
}

auto MMU::unmapped_io_read(const Address& address) const -> u8 {
    log_warn("Attempting to read from unused IO address 0x%x", address.value());
    return 0xFF;
}

void MMU::write(const Address& address, const u8 byte) {
    if (address.in_range(0x0000, 0x7FFF)) {
        gb.cartridge->write(address, byte);
        return;
    }

    // VRAM
    if (address.in_range(0x8000, 0x9FFF)) {
        // TODO: implement VRAM
        return;
    }

    // External (cartridge) RAM
    if (address.in_range(0xA000, 0xBFFF)) {
        gb.cartridge->write(address, byte);
        return;
    }

    // Internal work RAM
    if (address.in_range(0xC000, 0xDFFF)) {
        work_ram.at(address.value() - 0xC000) = byte;
        return;
    }

    // Mirrored RAM
    if (address.in_range(0xE000, 0xFDFF)) {
        log_warn("Attempting to write to mirrored work RAM");
        write(address.value() - 0x2000, byte);
        return;
    }

    // OAM
    if (address.in_range(0xFE00, 0xFE9F)) {
        oam_ram.at(address.value() - 0xC000) = byte;
        return;
    }

    if (address.in_range(0xFEA0, 0xFEFF)) {
        log_warn("Attempting to write to unusable memory 0x%x - 0x%x", address.value(), byte);
        return;
    }

    // Mapped IO
    if (address.in_range(0xFF00, 0xFF7F)) {
        write_io(address, byte);
        return;
    }

    // Zero Page ram
    if (address.in_range(0xFF80, 0xFFFE)) {
        high_ram.at(address.value() - 0xFF80) = byte;
        return;
    }

    // Interrupt Enable Register
    if (address == 0xFFFF) {
        // TODO: implement interrupt enable register
        return;
    }

    fatal_error("Attempted to write to unmapped memory address 0x0%X", address.value());
}

void MMU::write_io(const Address& address, const u8 byte) {
    // Implement hardware registers
    return;
}

void MMU::unmapped_io_write(const Address& address, const u8 byte) {
    log_warn("Attempting to write to unused IO address 0x%x - 0x%x", address.value(), byte);
}

auto MMU::boot_rom_active() const -> bool { return read(0xFF50) != 0x1; }

void MMU::dma_transfer(const u8 byte) {
    Address start_address = byte * 0x100;

    for (u8 i = 0x0; i <= 0x9F; i++) {
        Address from_address = start_address.value() + i;
        Address to_address = 0xFE00 + i;

        u8 value_at_address = read(from_address);
        write(to_address, value_at_address);
    }
}