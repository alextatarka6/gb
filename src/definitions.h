#pragma once

#include <cstdlib>
#include <cstdint>

using uint = unsigned int;

using u8 = u_int8_t;
using u16 = u_int16_t;
using s8 = int8_t;
using s16 = int16_t;

struct Noncopyable {
    auto operator=(const Noncopyable&) -> Noncopyable& = delete;
    Noncopyable(const Noncopyable&) = delete;
    Noncopyable() = default;
    ~Noncopyable() = default;
};