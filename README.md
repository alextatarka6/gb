# gb

A Game Boy (DMG) emulator written in C++17.

## Status

Work in progress. Currently implemented:

- CPU — most opcodes, interrupt handling
- Cartridge — No MBC, MBC1, MBC3
- MMU — memory map, DMA
- Video — background, window, sprites (DMG)
- APU — all four channels (CH1 square+sweep, CH2 square, CH3 wave, CH4 noise), frame sequencer, 44100 Hz stereo output
- Timer, serial, joypad input

Audio and video output are wired up internally. A platform backend (SDL2) is the next step to produce a playable build.

## Requirements

- CMake 3.5+
- A C++17 compiler (GCC or Clang)
- SDL2 *(optional — required for the graphical build)*

## Build

Run these from the **repository root** (`~/projects/gb`):

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

Or use the Makefile shortcuts from the repo root:

```bash
make          # release build
make debug    # debug build with compile_commands.json
make rebuild  # clean + release build
```

Binaries are written to `build/`.

## Run

### ROM info (no display required)

Prints cartridge header information — title, type, MBC, ROM/RAM size, licensee:

```bash
./build/gbemu-test <rom.gb>
```

### Graphical build (SDL2)

Install SDL2 (`sudo apt install libsdl2-dev` on Debian/Ubuntu, `brew install sdl2` on macOS), then rebuild. CMake will detect SDL2 and produce a `gbemu` binary:

```bash
./build/gbemu <rom.gb> [flags]
```

#### Flags

| Flag | Description |
|------|-------------|
| `--debug` | Enable the interactive debugger |
| `--trace` | Log every CPU instruction |
| `--silent` | Suppress all log output |
| `--headless` | Run without rendering frames |
| `--print-serial-output` | Print Game Boy serial port output (useful for test ROMs) |
| `--exit-on-infinite-jr` | Stop when an infinite `JR` loop is detected |

## Project layout

```
src/
├── apu/          # Audio Processing Unit (CH1–CH4, mixer, sample buffer)
├── cartridge/    # ROM parsing, No MBC / MBC1 / MBC3
├── cpu/          # LR35902 interpreter, opcode table
├── util/         # Logging, file I/O, bitwise helpers
├── video/        # PPU, framebuffer, tile/sprite rendering
├── gameboy.cc    # Top-level machine: wires CPU, APU, Video, Timer, MMU
└── mmu.cc        # Memory map, DMA
platforms/
├── test/         # gbemu-test binary (cartridge info only)
└── cli/          # Shared CLI argument parsing
```
