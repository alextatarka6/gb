#include "wave_channel.h"

void WaveChannel::write_nr30(u8 val) {
    dac_enabled_ = (val & 0x80) != 0;
    if (!dac_enabled_) channel_enabled_ = false;
}

void WaveChannel::write_nr31(u8 val) {
    length_counter_ = 256 - val;
}

void WaveChannel::write_nr32(u8 val) {
    output_level_ = (val >> 5) & 0x3;
}

void WaveChannel::write_nr33(u8 val) {
    freq_ = (freq_ & 0x700) | val;
}

void WaveChannel::write_nr34(u8 val) {
    length_enabled_ = (val >> 6) & 0x1;
    freq_ = (freq_ & 0xFF) | (static_cast<uint>(val & 0x7) << 8);
    if (val & 0x80) trigger();
}

u8 WaveChannel::read_nr30() const { return 0x7F | (dac_enabled_ ? 0x80 : 0); }
u8 WaveChannel::read_nr31() const { return 0xFF; }  // write-only
u8 WaveChannel::read_nr32() const { return 0x9F | (output_level_ << 5); }
u8 WaveChannel::read_nr33() const { return 0xFF; }  // write-only
u8 WaveChannel::read_nr34() const { return 0xBF | (length_enabled_ ? 0x40 : 0); }

void WaveChannel::write_wave_ram(u8 offset, u8 val) { wave_ram_[offset] = val; }
u8   WaveChannel::read_wave_ram(u8 offset)   const  { return wave_ram_[offset]; }

void WaveChannel::tick(int cycles) {
    if (!channel_enabled_) return;
    freq_timer_ -= cycles;
    while (freq_timer_ <= 0) {
        position_   = (position_ + 1) & 31;
        freq_timer_ += static_cast<int>(2048 - freq_) * 2;
    }
}

void WaveChannel::clock_length() {
    if (length_enabled_ && length_counter_ > 0) {
        if (--length_counter_ == 0) channel_enabled_ = false;
    }
}

float WaveChannel::output() const {
    if (!channel_enabled_ || !dac_enabled_ || output_level_ == 0) return 0.0f;
    u8 byte   = wave_ram_[position_ / 2];
    u8 nibble = (position_ & 1) ? (byte & 0x0F) : (byte >> 4);
    // shift right by 0, 1, or 2 for levels 1–3; mute (0) already handled above.
    static const u8 SHIFTS[4] = {4, 0, 1, 2};
    return static_cast<float>(nibble >> SHIFTS[output_level_]) / 15.0f;
}

bool WaveChannel::is_enabled() const { return channel_enabled_; }

void WaveChannel::trigger() {
    if (!dac_enabled_) return;
    channel_enabled_ = true;
    if (length_counter_ == 0) length_counter_ = 256;
    freq_timer_ = static_cast<int>(2048 - freq_) * 2;
    position_   = 0;
}
