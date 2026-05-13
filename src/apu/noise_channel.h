#pragma once

#include "../definitions.h"

// CH4 — noise channel, driven by a 15-bit (or 7-bit) linear feedback shift register.
class NoiseChannel {
public:
    void write_nr41(u8 val);  // Length load
    void write_nr42(u8 val);  // Volume envelope
    void write_nr43(u8 val);  // Clock shift / width mode / divisor code
    void write_nr44(u8 val);  // Length enable + trigger

    u8 read_nr41() const;
    u8 read_nr42() const;
    u8 read_nr43() const;
    u8 read_nr44() const;

    void tick(int cycles);
    void clock_length();
    void clock_envelope();

    float output() const;
    bool is_enabled() const;

private:
    void trigger();
    int timer_period() const;

    bool channel_enabled_ = false;

    uint length_counter_ = 0;
    bool length_enabled_ = false;

    u8   env_vol_    = 0;
    bool env_add_    = false;
    u8   env_period_ = 0;
    u8   env_timer_  = 0;
    u8   volume_     = 0;

    u8   clock_shift_  = 0;
    bool width_mode_   = false;  // false = 15-bit LFSR, true = 7-bit
    u8   divisor_code_ = 0;

    int  freq_timer_ = 0;
    u16  lfsr_       = 0x7FFF;
};
