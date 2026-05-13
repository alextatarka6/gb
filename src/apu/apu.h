#pragma once

#include "square_channel.h"
#include "wave_channel.h"
#include "noise_channel.h"
#include "audio_buffer.h"
#include "../definitions.h"
#include "../address.h"

// Top-level Audio Processing Unit.
// Mirrors the Video class pattern: owned by Gameboy, ticked each CPU step,
// registers routed through MMU read_io/write_io.
class APU {
public:
    APU();

    // Called after each CPU step with the number of T-cycles elapsed.
    void tick(int cycles);

    // Register access for the MMU (0xFF10–0xFF3F).
    u8   read(const Address& addr) const;
    void write(const Address& addr, u8 byte);

    // Platform layer drains samples from here each frame.
    AudioBuffer& get_buffer();

private:
    void clock_frame_sequencer();
    void push_sample();

    SquareChannel ch1_;  // CH1: square + sweep
    SquareChannel ch2_;  // CH2: square
    WaveChannel   ch3_;  // CH3: programmable wave
    NoiseChannel  ch4_;  // CH4: noise

    AudioBuffer buffer_;

    u8   nr50_        = 0;      // master volume
    u8   nr51_        = 0;      // left/right panning per channel
    bool apu_enabled_ = false;  // NR52 bit 7

    // 512 Hz frame sequencer (one step every 8192 T-cycles).
    uint frame_seq_counter_ = 0;
    uint frame_seq_step_    = 0;

    // Down-sampler: accumulate T-cycles and emit one stereo pair at ~44100 Hz.
    double sample_timer_              = 0.0;
    static constexpr double CYCLES_PER_SAMPLE = 4194304.0 / 44100.0;
};
