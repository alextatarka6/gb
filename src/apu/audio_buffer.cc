#include "audio_buffer.h"

void AudioBuffer::push(float left, float right) {
    samples_.push_back(left);
    samples_.push_back(right);
}

void AudioBuffer::clear() { samples_.clear(); }

size_t AudioBuffer::size() const { return samples_.size() / 2; }

const float* AudioBuffer::data() const { return samples_.data(); }
