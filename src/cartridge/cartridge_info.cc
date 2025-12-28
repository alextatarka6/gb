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
    u8 new_license_code_high = rom[header::new_license_code_high];
    u8 new_license_code_low = rom[header::new_license_code_low];

    info->type = get_type(type_code);
    info->version = version_code;
    info->rom_size = get_rom_size(rom_size_code);
    info->ram_size = get_ram_size(ram_size_code);
    info->title = get_title(rom);
    info->license = get_license(old_license_code, new_license_code_high, new_license_code_low);

    log_info("Title:\t\t %s (version %d)", info->title.c_str(), info->version);
    log_info("License:\t\t %s", info->license.c_str());
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

auto get_license(u8 old_license, u8 new_high, u8 new_low) -> std::string {
    switch (old_license) {
        case 0x00: return "None";
        case 0x01: return "Nintendo";
        case 0x08: return "Capcom";
        case 0x09: return "HOT-B";
        case 0x0A: return "Jaleco";
        case 0x0B: return "Coconuts Japan";
        case 0x0C: return "Elite Systems";
        case 0x13: return "EA (Electronic Arts)";
        case 0x18: return "Hudson Soft";
        case 0x19: return "ITC Entertainment";
        case 0x1A: return "Yanoman";
        case 0x1D: return "Japan Clary";
        case 0x1F: return "Virgin Games Ltd.";
        case 0x24: return "PCM Complete";
        case 0x25: return "San-X";
        case 0x28: return "Kemco";
        case 0x29: return "SETA Corporation";
        case 0x30: return "Infogrames";
        case 0x31: return "Nintendo";
        case 0x32: return "Bandai";

        case 0x33:
            return get_new_license(new_high, new_low);

        case 0x34: return "Konami";
        case 0x35: return "HectorSoft";
        case 0x38: return "Capcom";
        case 0x39: return "Banpresto";
        case 0x3C: return "Entertainment Interactive";
        case 0x3E: return "Gremlin";
        case 0x41: return "Ubi Soft";
        case 0x42: return "Atlus";
        case 0x44: return "Malibu Interactive";
        case 0x46: return "Angel";
        case 0x47: return "Spectrum HoloByte";
        case 0x49: return "Irem";
        case 0x4A: return "Virgin Games Ltd.";
        case 0x4D: return "Malibu Interactive";
        case 0x4F: return "U.S. Gold";
        case 0x50: return "Absolute";
        case 0x51: return "Acclaim Entertainment";
        case 0x52: return "Activision";
        case 0x53: return "Sammy USA Corporation";
        case 0x54: return "GameTek";
        case 0x55: return "Park Place";
        case 0x56: return "LJN";
        case 0x57: return "Matchbox";
        case 0x59: return "Milton Bradley Company";
        case 0x5A: return "Mindscape";
        case 0x5B: return "Romstar";
        case 0x5C: return "Naxat Soft";
        case 0x5D: return "Tradewest";
        case 0x60: return "Titus Interactive";
        case 0x61: return "Virgin Games Ltd.";
        case 0x67: return "Ocean Software";
        case 0x69: return "EA (Electronic Arts)";
        case 0x6E: return "Elite Systems";
        case 0x6F: return "Electro Brain";
        case 0x70: return "Infogrames";
        case 0x71: return "Interplay Entertainment";
        case 0x72: return "Broderbund";
        case 0x73: return "Sculptured Software";
        case 0x75: return "The Sales Curve Limited";
        case 0x78: return "THQ";
        case 0x79: return "Accolade";
        case 0x7A: return "Triffix Entertainment";
        case 0x7C: return "MicroProse";
        case 0x7F: return "Kemco";
        case 0x80: return "Misawa Entertainment";
        case 0x83: return "LOZC G.";
        case 0x86: return "Tokuma Shoten";
        case 0x8B: return "Bullet-Proof Software";
        case 0x8C: return "Vic Tokai";
        case 0x8E: return "Ape Inc.";
        case 0x8F: return "I'Max";
        case 0x91: return "Chunsoft";
        case 0x92: return "Video System";
        case 0x93: return "Tsubaraya Productions";
        case 0x95: return "Varie";
        case 0x96: return "Yonezawa/S'Pal";
        case 0x97: return "Kemco";
        case 0x99: return "Arc";
        case 0x9A: return "Nihon Bussan";
        case 0x9B: return "Tecmo";
        case 0x9C: return "Imagineer";
        case 0x9D: return "Banpresto";
        case 0x9F: return "Nova";
        case 0xA1: return "Hori Electric";
        case 0xA2: return "Bandai";
        case 0xA4: return "Konami";
        case 0xA6: return "Kawada";
        case 0xA7: return "Takara";
        case 0xA9: return "Technos Japan";
        case 0xAA: return "Broderbund";
        case 0xAC: return "Toei Animation";
        case 0xAD: return "Toho";
        case 0xAF: return "Namco";
        case 0xB0: return "Acclaim Entertainment";
        case 0xB1: return "ASCII / Nexsoft";
        case 0xB2: return "Bandai";
        case 0xB4: return "Square";
        case 0xB6: return "HAL Laboratory";
        case 0xB7: return "SNK";
        case 0xB9: return "Pony Canyon";
        case 0xBA: return "Culture Brain";
        case 0xBB: return "Sunsoft";
        case 0xBD: return "Sony Imagesoft";
        case 0xBF: return "Sammy Corporation";
        case 0xC0: return "Taito";
        case 0xC2: return "Kemco";
        case 0xC3: return "Square";
        case 0xC4: return "Tokuma Shoten";
        case 0xC5: return "Data East";
        case 0xC6: return "Tonkin House";
        case 0xC8: return "Koei";
        case 0xC9: return "UFL";
        case 0xCA: return "Ultra Games";
        case 0xCB: return "VAP";
        case 0xCC: return "Use Corporation";
        case 0xCD: return "Meldac";
        case 0xCE: return "Pony Canyon";
        case 0xCF: return "Angel";
        case 0xD0: return "Taito";
        case 0xD1: return "SOFEL";
        case 0xD2: return "Quest";
        case 0xD3: return "Sigma Enterprises";
        case 0xD4: return "ASK Kodansha";
        case 0xD6: return "Naxat Soft";
        case 0xD7: return "Copya System";
        case 0xD9: return "Banpresto";
        case 0xDA: return "Tomy";
        case 0xDB: return "LJN";
        case 0xDD: return "Nippon Computer Systems";
        case 0xDE: return "Human Entertainment";
        case 0xDF: return "Altron";
        case 0xE0: return "Jaleco";
        case 0xE1: return "Towa Chiki";
        case 0xE2: return "Yutaka";
        case 0xE3: return "Varie";
        case 0xE5: return "Epoch";
        case 0xE7: return "Athena";
        case 0xE8: return "Asmik Ace Entertainment";
        case 0xE9: return "Natsume";
        case 0xEA: return "King Records";
        case 0xEB: return "Atlus";
        case 0xEC: return "Epic/Sony Records";
        case 0xEE: return "IGS";
        case 0xF0: return "A Wave";
        case 0xF3: return "Extreme Entertainment";
        case 0xFF: return "LJN";

        default:
            log_error("Unknown old license code: %02X", old_license);
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
    std::string title;
    title.reserve(TITLE_LENGTH);

    for (u8 i = 0; i < TITLE_LENGTH; i++) {
        u8 byte = rom[header::title + i];

        // Title is null-terminated
        if (byte == 0x00) {
            break;
        }

        // Only accept printable ASCII (defensive for later carts)
        if (byte < 0x20 || byte > 0x7E) {
            break;
        }

        title.push_back(static_cast<char>(byte));
    }

    return title;
}