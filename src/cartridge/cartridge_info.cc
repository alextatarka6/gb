#include "cartridge_info.h"

auto get_info(std::vector<u8> rom) -> std::unique_ptr<CartridgeInfo> {
    std::unique_ptr<CartridgeInfo> info = std::make_unique<CartridgeInfo>();

    u8 type_code = rom[header::cartridge_type];
    u8 version_code = rom[header::version_number];
    u8 rom_size_code = rom[header::rom_size];
    u8 ram_size_code = rom[header::ram_size];

    info->type = get_type(type_code);
    info->version = version_code;
    info->rom_size = get_rom_size(rom_size_code);
    info->ram_size = get_ram_size(ram_size_code);
    info->title = get_title(rom);

    return info;
}