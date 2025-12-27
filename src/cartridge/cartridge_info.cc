#include "cartridge_info.h"

#include "../util/log.h"

#include <unordered_map>

auto get_info(std::vector<u8> rom) -> std::unique_ptr<CartridgeInfo> {
    std::unique_ptr<CartridgeInfo> info = std::make_unique<CartridgeInfo>();

    u8 type_code = rom[header::cartridge_type];
    u8 version_code = rom[header::version_number];
    u8 rom_size_code = rom[header::rom_size];
    u8 ram_size_code = rom[header::ram_size];
    u8 old_license_code = rom[header::old_license_code];
    u16 new_license_code_high = rom[header::new_license_code_high];
    u16 new_license_code_low = rom[header::new_license_code_low];

    info->type = get_type(type_code);
    info->version = version_code;
    info->rom_size = get_rom_size(rom_size_code);
    info->ram_size = get_ram_size(ram_size_code);
    info->title = get_title(rom);

    log_info("Title:\t\t %s (version %d)", info->title.c_str(), info->version);
    log_info("Cartridge:\t\t %s", describe(info->type).c_str());
    log_info("ROM Size:\t\t %s", describe(info->rom_size).c_str());
    log_info("RAM Size:\t\t %s", describe(info->ram_size).c_str());
    log_info("");

    return info;
}

auto get_type(u8 type) -> CartridgeType {
    switch(type) {
        case 0x00:
        case 0x08:
        case 0x09:
            return CartridgeType::ROMOnly;

        case 0x01:
        case 0x02:
        case 0x03:
        case 0xFF:
            return CartridgeType::MBC1;

        case 0x05:
        case 0x06:
            return CartridgeType::MBC2;

        case 0x0F:
        case 0x10:
        case 0x11:
        case 0x12:
        case 0x13:
            return CartridgeType::MBC3;

        case 0x15:
        case 0x16:
        case 0x17:
            return CartridgeType::MBC4;

        case 0x19:
        case 0x1A:
        case 0x1B:
        case 0x1C:
        case 0x1D:
        case 0x1E:
            return CartridgeType::MBC5;

        case 0x0B:
        case 0x0C:
        case 0x0D:
        case 0x20:
        case 0x22:
        case 0xFC:
        case 0xFD:
        case 0xFE:
            return CartridgeType::UNKNOWN;

        default:
            log_error("Unknown cartridge type: %X", type);
            return CartridgeType::UNKNOWN;
    }
}

auto describe(CartridgeType type) -> std::string {
    switch(type) {
        case CartridgeType::ROMOnly:
            return "ROM Only";
        case CartridgeType::MBC1:
            return "MBC1";
        case CartridgeType::MBC2:
            return "MBC2";
        case CartridgeType::MBC3:
            return "MBC3";
        case CartridgeType::MBC4:
            return "MBC4";
        case CartridgeType::MBC5:
            return "MBC5";
        case CartridgeType::UNKNOWN:
        default:
            return "Unknown";
    }
}

auto get_license(u16 old_license, u8 new_license_high, u8 new_license_low) -> std::string {
    // TODO
    switch(old_license) {
        case 0x00:
            return "None";
        case 0x01:
            return "Nintendo";
        case 0x33:
            return get_new_license(new_license_high, new_license_low);
        case 0x79:
            return "Accolade";
        case 0xA4:
            return "Konami";
        default:
            log_error("Unknown license code: %X", old_license);
            return "Unknown";
    }
}

auto get_new_license(u8 high, u8 low) -> std::string {
    std::string code;
    code.push_back(static_cast<char>(high));
    code.push_back(static_cast<char>(low));

    static const std::unordered_map<std::string, std::string> kNew = {
        {"00", "None"},
        {"01", "Nintendo Research & Development 1"},
        {"08", "Capcom"},
        {"13", "EA (Electronic Arts)"},
        {"18", "Hudson Soft"},
        {"19", "B-AI"},
        {"20", "KSS"},
        {"22", "Planning Office WADA"},
        {"24", "PCM Complete"},
        {"25", "San-X"},
        {"28", "Kemco"},
        {"29", "SETA Corporation"},
        {"30", "Viacom"},
        {"31", "Nintendo"},
        {"32", "Bandai"},
        {"33", "Ocean Software/Acclaim Entertainment"},
        {"34", "Konami"},
        {"35", "HectorSoft"},
        {"37", "Taito"},
        {"38", "Hudson Soft"},
        {"39", "Banpresto"},
        {"41", "Ubi Soft"},
        {"42", "Atlus"},
        {"44", "Malibu Interactive"},
        {"46", "Angel"},
        {"47", "Bullet-Proof Software"},
        {"49", "Irem"},
        {"50", "Absolute"},
        {"51", "Acclaim Entertainment"},
        {"52", "Activision"},
        {"53", "Sammy USA Corporation"},
        {"54", "Konami"},
        {"55", "Hi Tech Expressions"},
        {"56", "LJN"},
        {"57", "Matchbox"},
        {"58", "Mattel"},
        {"59", "Milton Bradley Company"},
        {"60", "Titus Interactive"},
        {"61", "Virgin Games Ltd."},
        {"64", "Lucasfilm Games"},
        {"67", "Ocean Software"},
        {"69", "EA (Electronic Arts)"},
        {"70", "Infogrames"},
        {"71", "Interplay Entertainment"},
        {"72", "Broderbund"},
        {"73", "Sculptured Software"},
        {"75", "The Sales Curve Limited"},
        {"78", "THQ"},
        {"79", "Accolade"},
        {"80", "Misawa Entertainment"},
        {"83", "LOZC G."},
        {"86", "Tokuma Shoten"},
        {"87", "Tsukuda Original"},
        {"91", "Chunsoft Co."},
        {"92", "Video System"},
        {"93", "Ocean Software/Acclaim Entertainment"},
        {"95", "Varie"},
        {"96", "Yonezawa/S'Pal"},
        {"97", "Kaneko"},
        {"99", "Pack-In-Video"},
        {"9H", "Bottom Up"},
        {"A4", "Konami (Yu-Gi-Oh!)"},
        {"BL", "MTO"},
        {"DK", "Kodansha"},
    };

    auto it = kNew.find(code);
    if (it != kNew.end()) return it->second;
    return "Unknown (new=" + code + ")";
}

auto get_rom_size(u8 size_code) -> ROMSize {
    switch(size_code) {
        case 0x00:
            return ROMSize::KB32;
        case 0x01:
            return ROMSize::KB64;
        case 0x02:
            return ROMSize::KB128;
        case 0x03:
            return ROMSize::KB256;
        case 0x04:
            return ROMSize::KB512;
        case 0x05:
            return ROMSize::MB1;
        case 0x06:
            return ROMSize::MB2;
        case 0x52:
            return ROMSize::MB1p1;
        case 0x53:
            return ROMSize::MB1p2;
        case 0x54:
            return ROMSize::MB1p5;
        default:
            log_error("Unknown ROM size: %X", size_code);
            return ROMSize::KB32;
    }
}

auto describe(ROMSize size) -> std::string {
    switch (size) {
        case ROMSize::KB32:
            return "32KB (no ROM banking)";
        case ROMSize::KB64:
            return "64KB (4 banks)";
        case ROMSize::KB128:
            return "128KB (8 banks)";
        case ROMSize::KB256:
            return "256KB (16 banks)";
        case ROMSize::KB512:
            return "512KB (32 banks)";
        case ROMSize::MB1:
            return "1MB (64 banks)";
        case ROMSize::MB2:
            return "2MB (128 banks)";
        case ROMSize::MB1p1:
            return "1.1MB (72 banks)";
        case ROMSize::MB1p2:
            return "1.2MB (80 banks)";
        case ROMSize::MB1p5:
            return "1.5MB (96 banks)";
        default:
            return "Unknown ROM Size";
    }
}

auto get_ram_size(u8 size_code) -> RAMSize {
    switch (size_code) {
        case 0x00:
            return RAMSize::None;
        case 0x01:
            return RAMSize::KB2;
        case 0x02:
            return RAMSize::KB8;
        case 0x03:
            return RAMSize::KB32;
        case 0x04:
            return RAMSize::KB128;
        case 0x05:
            return RAMSize::KB64;
        default:
            log_error("Unknown RAM size: %X", size_code);
            return RAMSize::None;
    }
}

auto get_actual_ram_size(RAMSize size) -> uint {
    switch(size) {
        case RAMSize::None:
            return 0x0;
        case RAMSize::KB2:
            return 0x800;
        case RAMSize::KB8:
            return 0x2000;
        case RAMSize::KB32:
            return 0x8000;
        case RAMSize::KB128:
            return 0x20000;
        case RAMSize::KB64:
            return 0x10000;
        default:
            return 0x0;
    }
}

auto describe(RAMSize size) -> std::string {
    switch(size) {
        case RAMSize::None:
            return "No RAM";
        case RAMSize::KB2:
            return "2KB";
        case RAMSize::KB8:
            return "8KB";
        case RAMSize::KB32:
            return "32KB";
        case RAMSize::KB128:
            return "128KB";
        case RAMSize::KB64:
            return "64KB";
        default:
            return "Unknown RAM Size";
    }
}

auto get_destination(u8 destination) -> Destination {
    switch (destination) {
        case 0x00:
            return Destination::Japanese;
        case 0x01:
            return Destination::NonJapanese;
        default:
            log_error("Unknown destination: %X", destination);
            return Destination::NonJapanese;
    }
}

auto describe(Destination destination) -> std::string {
    switch(destination) {
        case Destination::Japanese:
            return "Japanese";
        case Destination::NonJapanese:
        default:
            return "Non-Japanese";
    }
}

auto get_title(std::vector<u8>& rom) -> std::string {
    char name[TITLE_LENGTH] = {0};

    for (u8 i = 0; i < TITLE_LENGTH; i++) {
        name[i] = static_cast<char>(rom[header::title + i]);
    }

    return std::string(name);
}