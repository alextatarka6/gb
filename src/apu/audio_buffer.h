#pragma once

#include <vector>
#include <cstddef>

// Interleaved stereo float sample buffer (L, R, L, R, ...) fed to the audio backend.
class AudioBuffer {
public:
    void push(float left, float right);
    void clear();

    size_t size() const;         // number of stereo pairs
    const float* data() const;  // interleaved L/R

private:
    std::vector<float> samples_;
};
