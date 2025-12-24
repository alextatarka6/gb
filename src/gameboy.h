#pragma once

#include "cpu/cpu.h"

#include <memory>
#include <functional>

class Gameboy {
public:

private:
    void tick();

    CPU cpu;
    friend class CPU;
};