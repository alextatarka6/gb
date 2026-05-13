#pragma once

#include "../definitions.h"

// Handles CH1 (square + frequency sweep) and CH2 (square, no sweep).
// Parameterised at construction with has_sweep.
class SquareChannel {
public:
    explicit SquareChannel(bool has_sweep);

    // Register writes (NRx0–NRx4 relative to the channel base).
    void write_nr0(u8 val);   // Sweep control — CH1 only
    void write_nr1(u8 val);   // Duty cycle + length load
    void write_nr2(u8 val);   // Volume envelope
    void write_nr3(u8 val);   // Frequency low byte
    void write_nr4(u8 val);   // Frequency high + length enable + trigger

    // Register reads
    u8 read_nr0() const;
    u8 read_nr1() const;
    u8 read_nr2() const;
    u8 read_nr3() const;
    u8 read_nr4() const;

    // Called each CPU step; cycles is the number of T-cycles elapsed.
    void tick(int cycles);

    // Frame-sequencer clocks (called by APU at the correct rates).
    void clock_length();    // 256 Hz
    void clock_sweep();     // 128 Hz, CH1 only
    void clock_envelope();  // 64 Hz

    float output() const;
    bool is_enabled() const;

private:
    void trigger();
    uint sweep_calc() const;

    const bool has_sweep_;

    // Sweep (CH1 only)
    u8   sweep_period_  = 0;
    bool sweep_negate_  = false;
    u8   sweep_shift_   = 0;
    u8   sweep_timer_   = 0;
    bool sweep_enabled_ = false;
    uint sweep_shadow_  = 0;

    // Duty / length
    u8   duty_           = 0;
    u8   duty_pos_       = 0;
    uint length_counter_ = 0;
    bool length_enabled_ = false;

    // Frequency
    uint freq_      = 0;
    int  freq_timer_ = 0;

    // Volume envelope
    u8   env_vol_    = 0;
    bool env_add_    = false;
    u8   env_period_ = 0;
    u8   env_timer_  = 0;
    u8   volume_     = 0;

    bool channel_enabled_ = false;
};
