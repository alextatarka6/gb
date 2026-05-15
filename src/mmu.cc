#include "mmu.h"
#include "boot.h"
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
            return bootDMG[address.value()];
        }
        return gb.cartridge->read(address);
    }

    // VRAM
    if (address.in_range(0x8000, 0x9FFF)) {
        return gb.video.read(address.value() - 0x8000);
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
        return gb.cpu.interrupt_enabled.value();
    }

    fatal_error("Attempted to read from unmapped memory address 0x%X", address.value());
}

auto MMU::read_io(const Address& address) const -> u8 {
    switch (address.value()) {
        case 0xFF00: return gb.input.get_input();
        case 0xFF01: return gb.serial.read();
        case 0xFF02: return 0x00;
        case 0xFF04: return gb.timer.get_divider();
        case 0xFF05: return gb.timer.get_timer();
        case 0xFF06: return gb.timer.get_timer_modulo();
        case 0xFF07: return gb.timer.get_timer_control();
        case 0xFF0F: return gb.cpu.interrupt_flag.value();

        // APU registers and wave RAM
        case 0xFF10: case 0xFF11: case 0xFF12: case 0xFF13: case 0xFF14:
        case 0xFF16: case 0xFF17: case 0xFF18: case 0xFF19:
        case 0xFF1A: case 0xFF1B: case 0xFF1C: case 0xFF1D: case 0xFF1E:
        case 0xFF20: case 0xFF21: case 0xFF22: case 0xFF23:
        case 0xFF24: case 0xFF25: case 0xFF26:
        case 0xFF30: case 0xFF31: case 0xFF32: case 0xFF33:
        case 0xFF34: case 0xFF35: case 0xFF36: case 0xFF37:
        case 0xFF38: case 0xFF39: case 0xFF3A: case 0xFF3B:
        case 0xFF3C: case 0xFF3D: case 0xFF3E: case 0xFF3F:
            return gb.apu.read(address);

        // Video registers
        case 0xFF40: return gb.video.lcd_control.value();
        case 0xFF41: return gb.video.lcd_status.value();
        case 0xFF42: return gb.video.scroll_y.value();
        case 0xFF43: return gb.video.scroll_x.value();
        case 0xFF44: return gb.video.line.value();
        case 0xFF45: return gb.video.ly_compare.value();
        case 0xFF46: return gb.video.dma_transfer.value();
        case 0xFF47: return gb.video.bg_palette.value();
        case 0xFF48: return gb.video.sprite_palette_0.value();
        case 0xFF49: return gb.video.sprite_palette_1.value();
        case 0xFF4A: return gb.video.window_y.value();
        case 0xFF4B: return gb.video.window_x.value();

        case 0xFF50: return disable_boot_rom_switch.value();

        default: return unmapped_io_read(address);
    }
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
        gb.video.write(address.value() - 0x8000, byte);
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
        write(address.value() - 0x2000, byte);
        return;
    }

    // OAM
    if (address.in_range(0xFE00, 0xFE9F)) {
        oam_ram.at(address.value() - 0xFE00) = byte;
        return;
    }

    if (address.in_range(0xFEA0, 0xFEFF)) {
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
        gb.cpu.interrupt_enabled.set(byte);
        return;
    }

    fatal_error("Attempted to write to unmapped memory address 0x0%X", address.value());
}

void MMU::write_io(const Address& address, const u8 byte) {
    switch (address.value()) {
        case 0xFF00: gb.input.write(byte); break;
        case 0xFF01: gb.serial.write(byte); break;
        case 0xFF02: gb.serial.write_control(byte); break;
        case 0xFF04: gb.timer.reset_divider(); break; // any write resets DIV
        case 0xFF05: gb.timer.set_timer(byte); break;
        case 0xFF06: gb.timer.set_timer_modulo(byte); break;
        case 0xFF07: gb.timer.set_timer_control(byte); break;
        case 0xFF0F: gb.cpu.interrupt_flag.set(byte); break;

        // APU registers and wave RAM
        case 0xFF10: case 0xFF11: case 0xFF12: case 0xFF13: case 0xFF14:
        case 0xFF16: case 0xFF17: case 0xFF18: case 0xFF19:
        case 0xFF1A: case 0xFF1B: case 0xFF1C: case 0xFF1D: case 0xFF1E:
        case 0xFF20: case 0xFF21: case 0xFF22: case 0xFF23:
        case 0xFF24: case 0xFF25: case 0xFF26:
        case 0xFF30: case 0xFF31: case 0xFF32: case 0xFF33:
        case 0xFF34: case 0xFF35: case 0xFF36: case 0xFF37:
        case 0xFF38: case 0xFF39: case 0xFF3A: case 0xFF3B:
        case 0xFF3C: case 0xFF3D: case 0xFF3E: case 0xFF3F:
            gb.apu.write(address, byte); break;

        // Video registers
        case 0xFF40: gb.video.lcd_control.set(byte);
            fprintf(stderr, "[LCDC] = 0x%02X\n", byte);
            break;
        case 0xFF41: gb.video.lcd_status.set(byte); break;
        case 0xFF42: gb.video.scroll_y.set(byte); break;
        case 0xFF43: gb.video.scroll_x.set(byte); break;
        case 0xFF44: break; // LY is read-only
        case 0xFF45: gb.video.ly_compare.set(byte); break;
        case 0xFF46: gb.video.dma_transfer.set(byte); dma_transfer(byte); break;
        case 0xFF47: gb.video.bg_palette.set(byte); break;
        case 0xFF48: gb.video.sprite_palette_0.set(byte); break;
        case 0xFF49: gb.video.sprite_palette_1.set(byte); break;
        case 0xFF4A: gb.video.window_y.set(byte); break;
        case 0xFF4B: gb.video.window_x.set(byte); break;

        case 0xFF50: disable_boot_rom_switch.set(byte); break;

        default: unmapped_io_write(address, byte); break;
    }
}

void MMU::unmapped_io_write(const Address& address, const u8 byte) {
    log_warn("Attempting to write to unused IO address 0x%x - 0x%x", address.value(), byte);
}

auto MMU::boot_rom_active() const -> bool { return disable_boot_rom_switch.value() != 0x1; }

void MMU::dma_transfer(const u8 byte) {
    Address start_address = byte * 0x100;

    for (u8 i = 0x0; i <= 0x9F; i++) {
        Address from_address = start_address.value() + i;
        Address to_address = 0xFE00 + i;

        u8 value_at_address = read(from_address);
        write(to_address, value_at_address);
    }
}