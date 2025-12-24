#include "src/cartridge/cartridge.h"
#include "src/bus/bus.h"

#include <iostream>
#include <string>

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " game.gb\n";
        return 1;
    }

    Cartridge cart;
    std::string err;

    if (!cart.load(argv[1], &err)) {
        std::cerr << err << "\n";
        return 1;
    }

    const CartInfo& info = cart.info();

    std::cout << "Title: " << info.title << "\n";
    std::cout << "Cart type: 0x" << std::hex << (int)info.cart_type << "\n";
    std::cout << "ROM size code: 0x" << std::hex << (int)info.rom_size_code << "\n";
    std::cout << "RAM size code: 0x" << std::hex << (int)info.ram_size_code << "\n";

    uint8_t entry = cart.read8(0x0100);
    std::cout << "Byte at 0x0100: 0x" << std::hex << (int)entry << std::dec << "\n";
}