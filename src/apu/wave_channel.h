#pragma once

#include "../definitions.h"
#include <array>

// CH3 — programmable wave channel, plays 32 4-bit samples from wave RAM.
class WaveChannel {
public:
    void write_nr30(u8 val);  // DAC on/off
    void write_nr31(u8 val);  // Length load
    void write_nr32(u8 val);  // Output level
    void write_nr33(u8 val);  // Frequency low
    void write_nr34(u8 val);  // Frequency high + length enable + trigger

    u8 read_nr30() const;
    u8 read_nr31() const;
    u8 read_nr32() const;
    u8 read_nr33() const;
    u8 read_nr34() const;

    void write_wave_ram(u8 offset, u8 val);
    u8   read_wave_ram(u8 offset) const;

    void tick(int cycles);
    void clock_length();

    float output() const;
    bool is_enabled() const;

private:
    void trigger();

    bool dac_enabled_     = false;
    bool channel_enabled_ = false;

    uint length_counter_ = 0;
    bool length_enabled_ = false;

    u8 output_level_ = 0;  // 0=mute, 1=100%, 2=50%, 3=25%

    uint freq_      = 0;
    int  freq_timer_ = 0;
    u8   position_  = 0;  // nibble index 0–31

    std::array<u8, 16> wave_ram_ = {};
};
