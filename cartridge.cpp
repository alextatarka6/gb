#include "cartridge.h"

#include <fstream>
#include <iterator>
#include <string>

bool Cartridge::load(const std::string& path, std::string* err) {
    std::ifstream f(path, std::ios::binary);
    if (!f) {
        if (err) *err = "failed to open ROM: " + path;
        return false;
    }

    // Read entire file into rom_
    rom_.assign(std::istreambuf_iterator<char>(f), 
                std::istreambuf_iterator<char>());
    
    // Minimum size check so header reads are safe
    if (rom_.size() < 0x150) {
        if (err) *err = "ROM too small (need at least 0x150 bytes), got " + std::to_string(rom_.size());
        return false;
    }

    parse_header();
    return true;
}

void Cartridge::parse_header() {
    std::string title;
    for (int i = 0x134; i <= 0x143; i++) {
        uint8_t c = rom_[i];
        if (c == 0) break;                  // end of string
        if (c < 32 || c > 126) break;       // stop if not printable ASCII
        title.push_back(static_cast<char>(c));
    }

    info_.title = title;

    info_.cart_type = rom_[0x147];
    info_.rom_size_code = rom_[0x148];
    info_.ram_size_code = rom_[0x149];
}

uint8_t Cartridge::read8(uint16_t addr) const {
    if (addr <= 0x7FFF && addr < rom_.size()) {
        // Basic ROM read, no MBC banking yet
        return rom_[addr];
    }
    return 0xFF;
}

