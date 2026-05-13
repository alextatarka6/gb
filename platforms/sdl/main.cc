#include <SDL2/SDL.h>

#include "../../src/gameboy_prelude.h"
#include "../../src/video/framebuffer.h"
#include "../../platforms/cli/cli.h"

static const int SCALE = 3;

// Classic DMG green palette
struct RGBA { u8 r, g, b; };
static constexpr RGBA GB_PALETTE[4] = {
    {155, 188,  15},  // White
    {139, 172,  15},  // Light gray
    { 48,  98,  48},  // Dark gray
    { 15,  56,  15},  // Black
};

static RGBA color_to_rgba(Color c) {
    switch (c) {
        case Color::White:     return GB_PALETTE[0];
        case Color::LightGray: return GB_PALETTE[1];
        case Color::DarkGray:  return GB_PALETTE[2];
        case Color::Black:     return GB_PALETTE[3];
    }
    return GB_PALETTE[0];
}

static void handle_key(Gameboy& gb, SDL_Keycode key, bool pressed) {
    auto act = [&](GbButton btn) {
        if (pressed) gb.button_pressed(btn);
        else         gb.button_released(btn);
    };
    switch (key) {
        case SDLK_UP:        act(GbButton::Up);     break;
        case SDLK_DOWN:      act(GbButton::Down);   break;
        case SDLK_LEFT:      act(GbButton::Left);   break;
        case SDLK_RIGHT:     act(GbButton::Right);  break;
        case SDLK_z:         act(GbButton::A);      break;
        case SDLK_x:         act(GbButton::B);      break;
        case SDLK_RETURN:    act(GbButton::Start);  break;
        case SDLK_BACKSPACE: act(GbButton::Select); break;
        default: break;
    }
}

int main(int argc, char* argv[]) {
    CliOptions opts = get_cli_options(argc, argv);
    auto rom = read_bytes(opts.filename);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow(
        "gbemu",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        GAMEBOY_WIDTH  * SCALE,
        GAMEBOY_HEIGHT * SCALE,
        0
    );
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    SDL_RenderSetLogicalSize(renderer, GAMEBOY_WIDTH, GAMEBOY_HEIGHT);

    // Streaming texture — one pixel per GB pixel, scaled by the renderer.
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGB24,
        SDL_TEXTUREACCESS_STREAMING,
        GAMEBOY_WIDTH, GAMEBOY_HEIGHT
    );

    // Audio: 44100 Hz stereo float, queued from the main thread.
    SDL_AudioSpec desired = {};
    desired.freq     = 44100;
    desired.format   = AUDIO_F32SYS;
    desired.channels = 2;
    desired.samples  = 1024;
    SDL_AudioSpec obtained = {};
    SDL_AudioDeviceID audio_dev = SDL_OpenAudioDevice(
        nullptr, 0, &desired, &obtained, 0
    );
    if (audio_dev == 0) {
        fprintf(stderr, "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
    } else {
        SDL_PauseAudioDevice(audio_dev, 0);
    }
    // Allow ~4 SDL buffer-lengths of queued audio before dropping samples.
    const Uint32 audio_queue_limit = obtained.size * 4;

    Gameboy gameboy(rom, opts.options);
    bool should_quit = false;

    gameboy.run(
        [&]() { return should_quit; },
        [&](const FrameBuffer& fb) {
            // --- Events ---
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    should_quit = true;
                } else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
                    bool down = (event.type == SDL_KEYDOWN);
                    if (down && event.key.keysym.sym == SDLK_ESCAPE) {
                        should_quit = true;
                    }
                    handle_key(gameboy, event.key.keysym.sym, down);
                }
            }

            // --- Render frame ---
            void* pixels;
            int pitch;
            SDL_LockTexture(texture, nullptr, &pixels, &pitch);
            for (uint y = 0; y < GAMEBOY_HEIGHT; ++y) {
                u8* row = static_cast<u8*>(pixels) + y * pitch;
                for (uint x = 0; x < GAMEBOY_WIDTH; ++x) {
                    RGBA c = color_to_rgba(fb.get_pixel(x, y));
                    u8* p  = row + x * 3;
                    p[0] = c.r;
                    p[1] = c.g;
                    p[2] = c.b;
                }
            }
            SDL_UnlockTexture(texture);
            SDL_RenderClear(renderer);
            SDL_RenderCopy(renderer, texture, nullptr, nullptr);
            SDL_RenderPresent(renderer);

            // --- Queue audio ---
            if (audio_dev != 0) {
                AudioBuffer& buf = gameboy.get_audio_buffer();
                if (buf.size() > 0) {
                    if (SDL_GetQueuedAudioSize(audio_dev) < audio_queue_limit) {
                        SDL_QueueAudio(
                            audio_dev,
                            buf.data(),
                            static_cast<Uint32>(buf.size() * 2 * sizeof(float))
                        );
                    }
                    buf.clear();
                }
            }
        }
    );

    if (audio_dev != 0) SDL_CloseAudioDevice(audio_dev);
    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
