#pragma once

#include "cartridge_info.h"
#include "../address.h"
#include "../register.h"

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

struct CartInfo {
    std::string title;
    uint8_t cart_type = 0;
    uint8_t rom_size_code = 0;
    uint8_t ram_size_code = 0;
};

class Cartridge {
public:
    bool load(const std::string& path, std::string* err = nullptr);

    uint8_t read8(uint16_t addr) const;

    const CartInfo& info() const { return info_; }
    const std::vector<uint8_t>& rom() const { return rom_; }

private:
    void parse_header();

    std::vector<uint8_t> rom_;
    CartInfo info_{};
};