#include "noise_channel.h"

// Clock divisors for NR43 divisor code 0–7.
// Code 0 uses divisor 8; codes 1–7 use code * 16.
static const int DIVISORS[8] = {8, 16, 32, 48, 64, 80, 96, 112};

void NoiseChannel::write_nr41(u8 val) {
    length_counter_ = 64 - (val & 0x3F);
}

void NoiseChannel::write_nr42(u8 val) {
    env_vol_    = (val >> 4) & 0xF;
    env_add_    = (val >> 3) & 0x1;
    env_period_ = val & 0x7;
    if ((val & 0xF8) == 0) channel_enabled_ = false;
}

void NoiseChannel::write_nr43(u8 val) {
    clock_shift_  = (val >> 4) & 0xF;
    width_mode_   = (val >> 3) & 0x1;
    divisor_code_ = val & 0x7;
}

void NoiseChannel::write_nr44(u8 val) {
    length_enabled_ = (val >> 6) & 0x1;
    if (val & 0x80) trigger();
}

u8 NoiseChannel::read_nr41() const { return 0xFF; }  // write-only
u8 NoiseChannel::read_nr42() const {
    return (env_vol_ << 4) | (env_add_ ? 0x08 : 0) | env_period_;
}
u8 NoiseChannel::read_nr43() const {
    return (clock_shift_ << 4) | (width_mode_ ? 0x08 : 0) | divisor_code_;
}
u8 NoiseChannel::read_nr44() const { return 0xBF | (length_enabled_ ? 0x40 : 0); }

void NoiseChannel::tick(int cycles) {
    if (!channel_enabled_) return;
    freq_timer_ -= cycles;
    while (freq_timer_ <= 0) {
        freq_timer_ += timer_period();
        // Clock the LFSR: XOR bits 0 and 1, shift right, feed back into bit 14.
        u8 xor_bit = (lfsr_ & 0x1) ^ ((lfsr_ >> 1) & 0x1);
        lfsr_ >>= 1;
        lfsr_ |= static_cast<u16>(xor_bit) << 14;
        if (width_mode_) {
            lfsr_ = (lfsr_ & ~(1 << 6)) | (static_cast<u16>(xor_bit) << 6);
        }
    }
}

void NoiseChannel::clock_length() {
    if (length_enabled_ && length_counter_ > 0) {
        if (--length_counter_ == 0) channel_enabled_ = false;
    }
}

void NoiseChannel::clock_envelope() {
    if (env_period_ == 0) return;
    if (env_timer_ > 0) --env_timer_;
    if (env_timer_ == 0) {
        env_timer_ = env_period_;
        if (env_add_ && volume_ < 15) ++volume_;
        else if (!env_add_ && volume_ > 0) --volume_;
    }
}

float NoiseChannel::output() const {
    if (!channel_enabled_) return 0.0f;
    // LFSR bit 0 == 1 → output 0 (silence); bit 0 == 0 → output volume.
    return (lfsr_ & 0x1) ? 0.0f : (volume_ / 15.0f);
}

bool NoiseChannel::is_enabled() const { return channel_enabled_; }

void NoiseChannel::trigger() {
    channel_enabled_ = true;
    if (length_counter_ == 0) length_counter_ = 64;
    env_timer_  = env_period_;
    volume_     = env_vol_;
    freq_timer_ = timer_period();
    lfsr_       = 0x7FFF;
    if (env_vol_ == 0 && !env_add_) channel_enabled_ = false;
}

int NoiseChannel::timer_period() const {
    return DIVISORS[divisor_code_] << clock_shift_;
}
