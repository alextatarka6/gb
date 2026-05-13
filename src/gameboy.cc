#include "gameboy.h"
#include "cartridge/cartridge.h"

Gameboy::Gameboy(const std::vector<u8>& cartridge_data, Options& options, const std::vector<u8>& save_data) :
    cartridge(get_cartridge(cartridge_data, save_data)),
    cpu(*this, options),
    video(*this, options),
    mmu(*this, options),
    timer(*this),
    serial(options),
    debugger(*this, options)
{
    log_set_level(options.disable_logs
        ? LogLevel::Error
        : (options.trace ? LogLevel::Trace : LogLevel::Info)
    );
}

void Gameboy::run(
    const should_close_callback_t& _should_close_callback,
    const vblank_callback_t& _vblank_callback)
{
    should_close_callback = _should_close_callback;
    video.register_vblank_callback(_vblank_callback);

    while (!should_close_callback()) {
        tick();
    }
}

void Gameboy::button_pressed(GbButton button) {
    input.button_pressed(button);
}

void Gameboy::button_released(GbButton button) {
    input.button_released(button);
}

void Gameboy::debug_toggle_background() {
    video.debug_disable_background = !video.debug_disable_background;
}

void Gameboy::debug_toggle_sprites() {
    video.debug_disable_sprites = !video.debug_disable_sprites;
}

void Gameboy::debug_toggle_window() {
    video.debug_disable_window = !video.debug_disable_window;
}

auto Gameboy::get_cartridge_ram() const -> const std::vector<u8>& {
    return cartridge->get_cartridge_ram();
}

auto Gameboy::get_audio_buffer() -> AudioBuffer& {
    return apu.get_buffer();
}

void Gameboy::tick() {
    Cycles cycles = cpu.tick();
    apu.tick(static_cast<int>(cycles.cycles));
    video.tick(cycles);
    timer.tick(cycles.cycles);
    elapsed_cycles += cycles.cycles;
}
