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

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
template <typename... T> void unused(T&&... unused_vars) {}
#pragma clang diagnostic pop

#define fatal_error(...) \
    log_error("Fatal error @ %s (line %d)", __PRETTY_FUNCTION__, __LINE__); \
    log_error(__VA_ARGS__); \
    exit(1);