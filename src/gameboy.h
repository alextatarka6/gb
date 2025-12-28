#pragma once

#include "cpu/cpu.h"
#include "options.h"
#include "util/log.h"
#include "cartridge/cartridge.h"
#include "mmu.h"

#include <memory>
#include <functional>

class Gameboy {
public:
    Gameboy(const std::vector<u8>& cartridge_data, Options& options, 
        const std::vector<u8>& save_data = {});

    auto get_cartridge_ram() const -> const std::vector<u8>&;

private:
    void tick();

    std::shared_ptr<Cartridge> cartridge;

    CPU cpu;
    friend class CPU;

    MMU mmu;
    friend class MMU;

    uint elapsed_cyles = 0;
};