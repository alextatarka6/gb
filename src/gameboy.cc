#include "gameboy.h"

Gameboy::Gameboy(const std::vector<u8>& cartridge_data, Options& options, const std::vector<u8>& save_data) :
    cartridge(get_cartridge(cartridge_data, save_data)),
    cpu(*this, options),
    mmu(*this, options)
{
    if (options.disable_logs) log_set_level(LogLevel::Error);

    log_set_level(options.trace
    ? LogLevel::Trace
    : LogLevel::Info
    );
}