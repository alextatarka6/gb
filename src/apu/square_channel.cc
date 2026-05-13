#include "square_channel.h"

// 8-step duty waveform table. Index [duty][step].
static const u8 DUTY_TABLE[4][8] = {
    {0, 0, 0, 0, 0, 0, 0, 1},  // 12.5 %
    {1, 0, 0, 0, 0, 0, 0, 1},  // 25 %
    {1, 0, 0, 0, 0, 1, 1, 1},  // 50 %
    {0, 1, 1, 1, 1, 1, 1, 0},  // 75 %
};

SquareChannel::SquareChannel(bool has_sweep) : has_sweep_(has_sweep) {}

void SquareChannel::write_nr0(u8 val) {
    if (!has_sweep_) return;
    sweep_period_ = (val >> 4) & 0x7;
    sweep_negate_ = (val >> 3) & 0x1;
    sweep_shift_  = val & 0x7;
}

void SquareChannel::write_nr1(u8 val) {
    duty_           = (val >> 6) & 0x3;
    length_counter_ = 64 - (val & 0x3F);
}

void SquareChannel::write_nr2(u8 val) {
    env_vol_    = (val >> 4) & 0xF;
    env_add_    = (val >> 3) & 0x1;
    env_period_ = val & 0x7;
    // DAC disabled when upper 5 bits are all zero.
    if ((val & 0xF8) == 0) channel_enabled_ = false;
}

void SquareChannel::write_nr3(u8 val) {
    freq_ = (freq_ & 0x700) | val;
}

void SquareChannel::write_nr4(u8 val) {
    length_enabled_ = (val >> 6) & 0x1;
    freq_ = (freq_ & 0xFF) | (static_cast<uint>(val & 0x7) << 8);
    if (val & 0x80) trigger();
}

u8 SquareChannel::read_nr0() const {
    if (!has_sweep_) return 0xFF;
    return 0x80
         | (sweep_period_ << 4)
         | (sweep_negate_ ? 0x08 : 0)
         | sweep_shift_;
}

u8 SquareChannel::read_nr1() const { return 0x3F | (duty_ << 6); }
u8 SquareChannel::read_nr2() const {
    return (env_vol_ << 4) | (env_add_ ? 0x08 : 0) | env_period_;
}
u8 SquareChannel::read_nr3() const { return 0xFF; }  // write-only
u8 SquareChannel::read_nr4() const { return 0xBF | (length_enabled_ ? 0x40 : 0); }

void SquareChannel::tick(int cycles) {
    freq_timer_ -= cycles;
    while (freq_timer_ <= 0) {
        duty_pos_   = (duty_pos_ + 1) & 7;
        freq_timer_ += static_cast<int>(2048 - freq_) * 4;
    }
}

void SquareChannel::clock_length() {
    if (length_enabled_ && length_counter_ > 0) {
        if (--length_counter_ == 0) channel_enabled_ = false;
    }
}

void SquareChannel::clock_sweep() {
    if (!has_sweep_) return;
    if (sweep_timer_ > 0) --sweep_timer_;
    if (sweep_timer_ == 0) {
        sweep_timer_ = (sweep_period_ > 0) ? sweep_period_ : 8;
        if (sweep_enabled_ && sweep_period_ > 0) {
            uint new_freq = sweep_calc();
            if (new_freq < 2048 && sweep_shift_ > 0) {
                sweep_shadow_ = new_freq;
                freq_         = new_freq;
            }
            // Second overflow check: disable if overflow.
            if (sweep_calc() >= 2048) channel_enabled_ = false;
        }
    }
}

void SquareChannel::clock_envelope() {
    if (env_period_ == 0) return;
    if (env_timer_ > 0) --env_timer_;
    if (env_timer_ == 0) {
        env_timer_ = env_period_;
        if (env_add_ && volume_ < 15) ++volume_;
        else if (!env_add_ && volume_ > 0) --volume_;
    }
}

float SquareChannel::output() const {
    if (!channel_enabled_) return 0.0f;
    return DUTY_TABLE[duty_][duty_pos_] ? (volume_ / 15.0f) : 0.0f;
}

bool SquareChannel::is_enabled() const { return channel_enabled_; }

void SquareChannel::trigger() {
    channel_enabled_ = true;
    if (length_counter_ == 0) length_counter_ = 64;
    freq_timer_ = static_cast<int>(2048 - freq_) * 4;
    env_timer_  = env_period_;
    volume_     = env_vol_;

    if (has_sweep_) {
        sweep_shadow_  = freq_;
        sweep_timer_   = (sweep_period_ > 0) ? sweep_period_ : 8;
        sweep_enabled_ = (sweep_period_ > 0 || sweep_shift_ > 0);
        if (sweep_shift_ > 0 && sweep_calc() >= 2048) channel_enabled_ = false;
    }

    // DAC off → channel stays off.
    if (env_vol_ == 0 && !env_add_) channel_enabled_ = false;
}

uint SquareChannel::sweep_calc() const {
    uint delta = sweep_shadow_ >> sweep_shift_;
    if (sweep_negate_) {
        return (sweep_shadow_ > delta) ? (sweep_shadow_ - delta) : 0;
    }
    return sweep_shadow_ + delta;
}
