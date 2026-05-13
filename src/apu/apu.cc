#include "apu.h"

APU::APU() : ch1_(true), ch2_(false) {}

void APU::tick(int cycles) {
    if (!apu_enabled_) return;

    // Frame sequencer: fires at 512 Hz (every 8192 T-cycles).
    frame_seq_counter_ += static_cast<uint>(cycles);
    while (frame_seq_counter_ >= 8192) {
        frame_seq_counter_ -= 8192;
        clock_frame_sequencer();
    }

    ch1_.tick(cycles);
    ch2_.tick(cycles);
    ch3_.tick(cycles);
    ch4_.tick(cycles);

    // Down-sample to 44100 Hz.
    sample_timer_ += cycles;
    while (sample_timer_ >= CYCLES_PER_SAMPLE) {
        sample_timer_ -= CYCLES_PER_SAMPLE;
        push_sample();
    }
}

void APU::clock_frame_sequencer() {
    // Steps 0, 2, 4, 6 → length counters (256 Hz)
    if ((frame_seq_step_ & 1) == 0) {
        ch1_.clock_length();
        ch2_.clock_length();
        ch3_.clock_length();
        ch4_.clock_length();
    }
    // Steps 2, 6 → CH1 frequency sweep (128 Hz)
    if (frame_seq_step_ == 2 || frame_seq_step_ == 6) {
        ch1_.clock_sweep();
    }
    // Step 7 → volume envelopes (64 Hz)
    if (frame_seq_step_ == 7) {
        ch1_.clock_envelope();
        ch2_.clock_envelope();
        ch4_.clock_envelope();
    }
    frame_seq_step_ = (frame_seq_step_ + 1) & 7;
}

void APU::push_sample() {
    float s1 = ch1_.output();
    float s2 = ch2_.output();
    float s3 = ch3_.output();
    float s4 = ch4_.output();

    // NR51 panning: bit4=CH1L, bit5=CH2L, bit6=CH3L, bit7=CH4L
    //               bit0=CH1R, bit1=CH2R, bit2=CH3R, bit3=CH4R
    float left  = 0.0f;
    float right = 0.0f;
    if (nr51_ & 0x10) left  += s1;
    if (nr51_ & 0x20) left  += s2;
    if (nr51_ & 0x40) left  += s3;
    if (nr51_ & 0x80) left  += s4;
    if (nr51_ & 0x01) right += s1;
    if (nr51_ & 0x02) right += s2;
    if (nr51_ & 0x04) right += s3;
    if (nr51_ & 0x08) right += s4;

    // Normalise to 4 channels then apply NR50 master volume (1–8).
    left  /= 4.0f;
    right /= 4.0f;
    float left_vol  = static_cast<float>(((nr50_ >> 4) & 0x7) + 1) / 8.0f;
    float right_vol = static_cast<float>((nr50_ & 0x7) + 1) / 8.0f;

    buffer_.push(left * left_vol, right * right_vol);
}

u8 APU::read(const Address& addr) const {
    u16 a = addr.value();

    switch (a) {
        case 0xFF10: return ch1_.read_nr0();
        case 0xFF11: return ch1_.read_nr1();
        case 0xFF12: return ch1_.read_nr2();
        case 0xFF13: return ch1_.read_nr3();
        case 0xFF14: return ch1_.read_nr4();

        case 0xFF15: return 0xFF;  // NR20 unused
        case 0xFF16: return ch2_.read_nr1();
        case 0xFF17: return ch2_.read_nr2();
        case 0xFF18: return ch2_.read_nr3();
        case 0xFF19: return ch2_.read_nr4();

        case 0xFF1A: return ch3_.read_nr30();
        case 0xFF1B: return ch3_.read_nr31();
        case 0xFF1C: return ch3_.read_nr32();
        case 0xFF1D: return ch3_.read_nr33();
        case 0xFF1E: return ch3_.read_nr34();

        case 0xFF1F: return 0xFF;  // NR40 unused
        case 0xFF20: return ch4_.read_nr41();
        case 0xFF21: return ch4_.read_nr42();
        case 0xFF22: return ch4_.read_nr43();
        case 0xFF23: return ch4_.read_nr44();

        case 0xFF24: return nr50_;
        case 0xFF25: return nr51_;
        case 0xFF26: {
            u8 status = apu_enabled_ ? 0x80 : 0;
            if (ch1_.is_enabled()) status |= 0x01;
            if (ch2_.is_enabled()) status |= 0x02;
            if (ch3_.is_enabled()) status |= 0x04;
            if (ch4_.is_enabled()) status |= 0x08;
            return status | 0x70;  // bits 4–6 read as 1
        }
        default: break;
    }

    // Wave RAM 0xFF30–0xFF3F
    if (a >= 0xFF30 && a <= 0xFF3F) {
        return ch3_.read_wave_ram(static_cast<u8>(a - 0xFF30));
    }

    return 0xFF;
}

void APU::write(const Address& addr, u8 byte) {
    u16 a = addr.value();

    // NR52 must be writable regardless of APU power state.
    if (a == 0xFF26) {
        bool was_enabled = apu_enabled_;
        apu_enabled_ = (byte & 0x80) != 0;
        if (was_enabled && !apu_enabled_) {
            // Power-off: reset all sound registers.
            nr50_ = 0;
            nr51_ = 0;
            ch1_.write_nr0(0); ch1_.write_nr1(0); ch1_.write_nr2(0);
            ch1_.write_nr3(0); ch1_.write_nr4(0);
            ch2_.write_nr1(0); ch2_.write_nr2(0);
            ch2_.write_nr3(0); ch2_.write_nr4(0);
            ch3_.write_nr30(0); ch3_.write_nr31(0); ch3_.write_nr32(0);
            ch3_.write_nr33(0); ch3_.write_nr34(0);
            ch4_.write_nr41(0); ch4_.write_nr42(0);
            ch4_.write_nr43(0); ch4_.write_nr44(0);
            frame_seq_counter_ = 0;
            frame_seq_step_    = 0;
        }
        return;
    }

    // Wave RAM is accessible even when APU is off.
    if (a >= 0xFF30 && a <= 0xFF3F) {
        ch3_.write_wave_ram(static_cast<u8>(a - 0xFF30), byte);
        return;
    }

    if (!apu_enabled_) return;

    switch (a) {
        case 0xFF10: ch1_.write_nr0(byte); break;
        case 0xFF11: ch1_.write_nr1(byte); break;
        case 0xFF12: ch1_.write_nr2(byte); break;
        case 0xFF13: ch1_.write_nr3(byte); break;
        case 0xFF14: ch1_.write_nr4(byte); break;

        case 0xFF16: ch2_.write_nr1(byte); break;
        case 0xFF17: ch2_.write_nr2(byte); break;
        case 0xFF18: ch2_.write_nr3(byte); break;
        case 0xFF19: ch2_.write_nr4(byte); break;

        case 0xFF1A: ch3_.write_nr30(byte); break;
        case 0xFF1B: ch3_.write_nr31(byte); break;
        case 0xFF1C: ch3_.write_nr32(byte); break;
        case 0xFF1D: ch3_.write_nr33(byte); break;
        case 0xFF1E: ch3_.write_nr34(byte); break;

        case 0xFF20: ch4_.write_nr41(byte); break;
        case 0xFF21: ch4_.write_nr42(byte); break;
        case 0xFF22: ch4_.write_nr43(byte); break;
        case 0xFF23: ch4_.write_nr44(byte); break;

        case 0xFF24: nr50_ = byte; break;
        case 0xFF25: nr51_ = byte; break;

        default: break;
    }
}

AudioBuffer& APU::get_buffer() { return buffer_; }
