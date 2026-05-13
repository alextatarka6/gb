# GB Emulator — Expansion Plan

## Overview

This plan covers two major additions:
1. **Audio (APU)** — implement the Game Boy's 4-channel audio processing unit in C++
2. **Full-stack web app** — compile the emulator to WebAssembly and build a website with ROM uploads, save states, and in-browser play

---

## Part 1: Audio (APU)

### Background

The Game Boy APU has four sound channels mixed to stereo output:
- **CH1** — Square wave with frequency sweep
- **CH2** — Square wave (no sweep)
- **CH3** — Programmable wave (32 4-bit samples in wave RAM)
- **CH4** — Noise (linear feedback shift register)

Each channel has its own length counter, volume envelope, and DAC. The final output is mixed via left/right panning registers (NR51) and a master volume register (NR50).

### Register Map

| Range | Channel | Registers |
|-------|---------|-----------|
| `0xFF10–0xFF14` | CH1 (Square + Sweep) | Sweep, Duty/Length, Volume Envelope, Freq Lo, Freq Hi/Trigger |
| `0xFF16–0xFF19` | CH2 (Square) | Duty/Length, Volume Envelope, Freq Lo, Freq Hi/Trigger |
| `0xFF1A–0xFF1E` | CH3 (Wave) | DAC On/Off, Length, Output Level, Freq Lo, Freq Hi/Trigger |
| `0xFF20–0xFF23` | CH4 (Noise) | Length, Volume Envelope, Freq/LFSR, Control/Trigger |
| `0xFF24` | NR50 | Master volume + VIN routing |
| `0xFF25` | NR51 | Left/right panning per channel |
| `0xFF26` | NR52 | Sound on/off + channel status (read-only bits) |
| `0xFF30–0xFF3F` | Wave RAM | 16 bytes = 32 × 4-bit samples for CH3 |

### Implementation Steps

#### Step 1 — Create `src/apu/` module

```
src/apu/
├── apu.h / apu.cc          # Top-level APU class (mirrors Video structure)
├── square_channel.h/.cc     # CH1 + CH2 (parameterized for sweep)
├── wave_channel.h/.cc       # CH3
├── noise_channel.h/.cc      # CH4
└── audio_buffer.h/.cc       # Sample ring-buffer fed to the audio backend
```

**`Apu` class responsibilities:**
- Handle all register reads/writes from MMU (`0xFF10–0xFF3F`)
- Call `tick(int cycles)` each CPU step (same pattern as `Video::tick`)
- Down-sample from ~4.19 MHz to output sample rate (44100 Hz) using a simple counter accumulator
- Expose `AudioBuffer& get_buffer()` for the platform layer to drain

#### Step 2 — Implement each channel

**Square channels (CH1, CH2):**
- 4-bit duty cycle register selects one of four waveforms (12.5%, 25%, 50%, 75%)
- Frequency from `(2048 - freq) * 4` CPU clocks per period
- Volume envelope: every N frames (1 frame = 8192 CPU cycles) add/subtract 1 from volume (0–15)
- CH1 only: frequency sweep (shift + add/subtract)
- Trigger (bit 7 of NRx4) restarts the channel and reloads length counter

**Wave channel (CH3):**
- Plays 32 nibbles from wave RAM sequentially
- Output level shifts the nibble right by 0, 1, 2, or 4 bits
- Frequency from `(2048 - freq) * 2` CPU clocks per sample step

**Noise channel (CH4):**
- 15-bit or 7-bit LFSR generates pseudo-random output
- Clock divider + shift amount control pitch
- Same volume envelope as CH1/CH2

#### Step 3 — Frame sequencer

A 512 Hz internal clock (every 8192 CPU cycles) drives:
- **256 Hz** — Length counters (disable channel when counter hits 0)
- **128 Hz** — CH1 frequency sweep
- **64 Hz** — Volume envelopes

Implement as a step counter (`frame_seq_step` 0–7) incremented each 8192-cycle tick.

#### Step 4 — Mixing & output

```
sample = (ch1 + ch2 + ch3 + ch4) / 4   // mono mix
left  = mix of channels enabled in NR51 * NR50 left volume
right = mix of channels enabled in NR51 * NR50 right volume
```

Samples are 32-bit floats in `[-1.0, 1.0]`. Down-sample by accumulating a fractional counter each CPU cycle and emitting a sample when it crosses 1.0.

#### Step 5 — Wire into `Gameboy` and `MMU`

- `Gameboy` owns an `Apu` instance alongside `Video`, `Timer`, etc.
- `Gameboy::tick()` calls `apu_.tick(cycles)` after the CPU step
- `MMU::read_byte` / `write_byte` routes `0xFF10–0xFF3F` to `apu_`

#### Step 6 — Platform audio output

For the native SDL2 build, use `SDL_OpenAudioDevice` with a callback that drains `AudioBuffer`. For the WebAssembly build, expose a JS-callable function that copies the buffer into a Web Audio `AudioWorkletProcessor`.

---

## Part 2: Full-Stack Website

### Architecture Overview

```
┌──────────────────────────────────────────────────────┐
│                    Browser                           │
│  React frontend  ←→  WASM emulator core              │
│  (ROM picker, save states, gamepad, display)         │
└──────────┬───────────────────────┬───────────────────┘
           │ Supabase JS client    │ (validation only)
           │ (storage + DB calls)  │ REST
           ▼                       ▼
┌──────────────────────┐  ┌────────────────────────────┐
│     Supabase         │  │  Express backend (thin)    │
│                      │  │  - ROM header validation   │
│  Storage buckets:    │  │  - Rate limiting           │
│    roms/             │  │  (optional — can omit if   │
│    save-states/      │  │   validation done in-browser│
│                      │  └────────────────────────────┘
│  Postgres tables:    │
│    roms              │
│    save_states       │
│                      │
│  Auth (optional):    │
│    per-user RLS      │
└──────────────────────┘
```

The frontend calls Supabase directly using `@supabase/supabase-js` — no backend is required for normal CRUD and file operations. A thin Express backend is optional and only needed for server-side ROM validation; that logic can also be done in the browser, making the stack entirely frontend + Supabase.

### Stack

| Layer | Technology |
|-------|-----------|
| Emulator compilation | Emscripten (C++ → WASM) |
| Frontend framework | React + TypeScript |
| Frontend build | Vite |
| Styling | Tailwind CSS |
| Backend (optional) | Node.js + Express — ROM validation + rate limiting only |
| Database | Supabase Postgres |
| File storage | Supabase Storage (`roms` + `save-states` buckets) |
| Auth (optional) | Supabase Auth — gate save states per user with RLS |

---

### Phase 1 — Emscripten WASM build

#### 1.1 — Add Emscripten CMake target

Create `platforms/web/CMakeLists.txt`:
- Compile `gbemu-core` + a thin JS-bindings layer with Emscripten
- Output `gbemu.js` + `gbemu.wasm`
- Enable `ASYNCIFY` if needed for the game loop; otherwise use `EMSCRIPTEN_KEEPALIVE` exports

Key exported C functions (defined in `platforms/web/web_bindings.cc`):

```cpp
// Lifecycle
EMSCRIPTEN_KEEPALIVE void gb_create(const uint8_t* rom, size_t len);
EMSCRIPTEN_KEEPALIVE void gb_destroy();
EMSCRIPTEN_KEEPALIVE void gb_reset();

// Execution
EMSCRIPTEN_KEEPALIVE void gb_run_frame();   // advance one full 70224-cycle frame

// Video
EMSCRIPTEN_KEEPALIVE const uint8_t* gb_get_framebuffer(); // 160*144*4 RGBA bytes

// Audio
EMSCRIPTEN_KEEPALIVE const float* gb_get_audio_buffer();
EMSCRIPTEN_KEEPALIVE int gb_get_audio_samples();
EMSCRIPTEN_KEEPALIVE void gb_clear_audio_buffer();

// Input
EMSCRIPTEN_KEEPALIVE void gb_set_input(uint8_t buttons);  // bitmask

// Save states
EMSCRIPTEN_KEEPALIVE size_t gb_save_state(uint8_t* out, size_t max_len);
EMSCRIPTEN_KEEPALIVE bool   gb_load_state(const uint8_t* data, size_t len);
```

#### 1.2 — Save state serialization

Add `Gameboy::serialize(std::vector<uint8_t>&)` and `Gameboy::deserialize(const uint8_t*)` that snapshot the entire machine state:
- All CPU registers + flags + cycle counter
- Full MMU RAM contents (work RAM, HRAM, OAM, VRAM)
- Cartridge RAM + current MBC bank registers
- APU channel state (envelopes, positions, LFSR state)
- Video controller state (mode, scanline counter, LCDC, palette)
- Timer state

Format: simple flat binary (or MessagePack for forward-compatibility). Prefix with a 4-byte magic + 2-byte version so the loader can reject stale saves.

#### 1.3 — Emscripten build script

Add `Makefile` targets:
```makefile
wasm:
    emcmake cmake -B build-web -DPLATFORM=web
    emmake make -C build-web
    cp build-web/gbemu.{js,wasm} web/public/
```

---

### Phase 2 — Frontend (`web/`)

Create a Vite + React + TypeScript project at `web/`.

#### Directory layout

```
web/
├── public/
│   ├── gbemu.js         # copied from WASM build
│   └── gbemu.wasm
├── src/
│   ├── App.tsx
│   ├── components/
│   │   ├── EmulatorCanvas.tsx   # 160×144 canvas + scaling
│   │   ├── AudioEngine.tsx      # Web Audio AudioWorklet bridge
│   │   ├── RomUploader.tsx      # drag-and-drop + file picker
│   │   ├── SaveStatePanel.tsx   # list, save, load, delete
│   │   ├── GamepadInput.tsx     # keyboard + gamepad mapping
│   │   └── Navbar.tsx
│   ├── hooks/
│   │   ├── useEmulator.ts       # WASM lifecycle + rAF game loop
│   │   ├── useAudio.ts          # AudioWorklet setup + buffer drain
│   │   └── useSaveStates.ts     # Supabase save state CRUD
│   ├── lib/
│   │   └── supabase.ts          # createClient singleton
│   └── types/
│       └── emulator.d.ts
├── index.html
├── vite.config.ts
├── tailwind.config.ts
└── tsconfig.json
```

#### Key component details

**`useEmulator.ts`**
- Loads `gbemu.js` via dynamic `import()`
- Calls `gb_create(romBytes)` on ROM load
- `requestAnimationFrame` loop calls `gb_run_frame()`, reads framebuffer, writes to canvas via `ImageData`
- Exposes `pause()`, `resume()`, `reset()`, `loadState(blob)`, `saveState(): Uint8Array`

**`EmulatorCanvas.tsx`**
- `<canvas>` sized 160×144, CSS-scaled to fill the viewport (integer scaling preferred: 2×, 3×, 4×)
- Draws each frame from the WASM framebuffer using `ctx.putImageData`

**`AudioEngine.tsx`**
- Registers an `AudioWorkletProcessor` (`gbemu-processor.js`) that receives samples from the main thread via a `SharedArrayBuffer` ring buffer (or `MessagePort` if SAB is unavailable)
- Drains `gb_get_audio_buffer()` in the rAF loop and posts to the worklet

**`RomUploader.tsx`**
- Drag-and-drop zone + `<input type="file" accept=".gb,.gbc">`
- On file select: read as `ArrayBuffer`, validate GB header magic at offset `0x0104` in the browser, then:
  1. Upload binary to Supabase Storage: `supabase.storage.from('roms').upload(id, file)`
  2. Insert metadata row: `supabase.from('roms').insert({ id, title, size })`
  3. Load bytes into WASM: `gb_create(romBytes)`
- Shows cartridge title parsed from header bytes

**`SaveStatePanel.tsx`**
- Lists saves via `supabase.from('save_states').select('*').eq('rom_id', currentRomId)`
- "Save" button:
  1. Call `gb_save_state()` → get `Uint8Array`
  2. Capture framebuffer as PNG thumbnail (toBlob on a temp canvas)
  3. Upload state blob: `supabase.storage.from('save-states').upload(id, blob)`
  4. Insert row with thumbnail as base64 inline (small enough at ~2 KB)
- "Load" button: `supabase.storage.from('save-states').download(path)` → pass bytes to `gb_load_state()`
- "Delete" button: remove Storage object + delete DB row in parallel

**`GamepadInput.tsx`**
- Keyboard map (configurable): Arrow keys, Z/X, Enter, Shift → GB buttons
- Gamepad API polling each rAF frame for controller support
- Calls `gb_set_input(bitmask)` each frame

---

### Phase 3 — Supabase setup

#### 3.1 — Storage buckets

Create two buckets in the Supabase dashboard (or via `supabase` CLI migrations):

| Bucket | Public? | Max file size | Allowed MIME types |
|--------|---------|---------------|--------------------|
| `roms` | false | 8 MB | `application/octet-stream` |
| `save-states` | false | 512 KB | `application/octet-stream` |

Set buckets to **private** so files are only accessible through signed URLs or service-role calls, not raw public URLs.

#### 3.2 — Postgres schema

Run in the Supabase SQL editor (or as a migration file):

```sql
create table roms (
  id          uuid primary key default gen_random_uuid(),
  user_id     uuid references auth.users(id),   -- null = anonymous
  title       text not null,
  size        integer not null,
  storage_path text not null,                   -- key in 'roms' bucket
  uploaded_at timestamptz not null default now()
);

create table save_states (
  id           uuid primary key default gen_random_uuid(),
  user_id      uuid references auth.users(id),
  rom_id       uuid not null references roms(id) on delete cascade,
  name         text not null,
  storage_path text not null,                   -- key in 'save-states' bucket
  thumbnail    text,                            -- base64 PNG (~2 KB inline)
  created_at   timestamptz not null default now()
);
```

#### 3.3 — Row Level Security (RLS)

Enable RLS on both tables. For an **anonymous / public** launch (no login required):

```sql
-- anyone can insert and read their own rows identified by a session token
alter table roms        enable row level security;
alter table save_states enable row level security;

-- public: allow all reads and inserts (tighten once auth is added)
create policy "public read roms"        on roms        for select using (true);
create policy "public insert roms"      on roms        for insert with check (true);
create policy "public read saves"       on save_states for select using (true);
create policy "public insert saves"     on save_states for insert with check (true);
create policy "public delete saves"     on save_states for delete using (true);
```

When Supabase Auth is added later, replace the `using (true)` policies with `using (auth.uid() = user_id)` to scope each user to their own data.

#### 3.4 — Supabase client (`web/src/lib/supabase.ts`)

```ts
import { createClient } from '@supabase/supabase-js'

export const supabase = createClient(
  import.meta.env.VITE_SUPABASE_URL,
  import.meta.env.VITE_SUPABASE_ANON_KEY
)
```

The anon key is safe to expose in the browser — RLS policies enforce access control.

#### 3.5 — `useSaveStates.ts` hook

```ts
// list saves for a ROM
const { data } = await supabase
  .from('save_states')
  .select('*')
  .eq('rom_id', romId)
  .order('created_at', { ascending: false })

// upload a new save
const path = `${romId}/${crypto.randomUUID()}.state`
await supabase.storage.from('save-states').upload(path, stateBlob)
await supabase.from('save_states').insert({ rom_id: romId, name, storage_path: path, thumbnail })

// load a save
const { data: blob } = await supabase.storage.from('save-states').download(storagePath)
const bytes = new Uint8Array(await blob.arrayBuffer())
gb_load_state(bytes)

// delete a save
await Promise.all([
  supabase.storage.from('save-states').remove([storagePath]),
  supabase.from('save_states').delete().eq('id', saveId)
])
```

#### 3.6 — Optional thin Express backend

If server-side validation is wanted, a minimal Express server handles only:
- ROM header validation (check `0x0104–0x0133` Nintendo logo bytes)
- Rate limiting (e.g., `express-rate-limit`: 10 ROM uploads/hour per IP)
- Proxying the upload to Supabase Storage with the service-role key (keeps service key off the client)

Otherwise skip the backend entirely — all Supabase calls go directly from the browser using the anon key + RLS.

#### 3.7 — Validation & limits

- ROM header: verify Nintendo logo bytes at `0x0104` client-side before uploading
- Storage bucket policies enforce the max file sizes (8 MB / 512 KB) on the Supabase side
- Supabase free tier: 1 GB Storage, 500 MB Postgres — more than enough for a personal/demo project

---

### Phase 4 — Deployment

#### Local dev

```bash
# .env.local (web/)
VITE_SUPABASE_URL=https://<project>.supabase.co
VITE_SUPABASE_ANON_KEY=<anon-key>
VITE_API_URL=http://localhost:3001   # only if using thin Express backend

# start frontend
cd web && npm run dev

# start backend (only if using thin Express layer)
cd server && npm run dev
```

#### Production — AWS

```
┌─────────────────────────────────────────────────────────────────┐
│  Users                                                          │
│    │                                                            │
│    ▼                                                            │
│  CloudFront (CDN + HTTPS)                                       │
│    │                              │                             │
│    ▼                              ▼                             │
│  AWS Amplify                  Elastic Beanstalk                 │
│  (React + WASM static site)   (Express backend — optional)      │
│                                   │                             │
│                               Supabase (external)               │
│                               - Postgres (metadata)             │
│                               - Storage (ROMs + save states)    │
└─────────────────────────────────────────────────────────────────┘
```

**AWS Amplify (frontend)**
- Connect the GitHub repo; Amplify auto-builds on push to `main`
- Build command: `cd web && npm ci && npm run build` (WASM artifacts must be pre-built and committed to `web/public/`, or built in the Amplify build step using an Emscripten Docker image)
- Output directory: `web/dist`
- Set environment variables (`VITE_SUPABASE_URL`, `VITE_SUPABASE_ANON_KEY`) in the Amplify console
- Amplify provides a default `*.amplifyapp.com` domain; attach a custom domain via Route 53

**CloudFront (CDN)**
- Create a CloudFront distribution in front of the Amplify URL (or use Amplify's built-in CDN — Amplify already sits behind CloudFront internally; for full control create a separate distribution)
- Add a cache behavior for `*.wasm` files: set `Content-Type: application/wasm`, long TTL (1 year), cache-bust by filename hash
- Enable `Cross-Origin-Opener-Policy` and `Cross-Origin-Embedder-Policy` response headers (required for `SharedArrayBuffer` / audio worklet) via a CloudFront Response Headers Policy

**Elastic Beanstalk (optional Express backend)**
- Only needed if the thin validation/rate-limiting backend is kept
- Dockerize the Express app (`server/Dockerfile`) and deploy to an EB environment (Node.js platform or Docker platform)
- EB environment variables: `SUPABASE_URL`, `SUPABASE_SERVICE_ROLE_KEY`
- Put EB behind the same CloudFront distribution under the `/api/*` path behavior to avoid CORS issues and share HTTPS termination
- Scale: single `t3.micro` instance is sufficient; EB auto-scaling handles bursts

**Supabase**
- No AWS hosting needed — Supabase is fully managed external SaaS
- Set allowed origins in Supabase Auth settings to the CloudFront/Amplify domain

**WASM build in CI**
- Add a GitHub Actions workflow that runs the Emscripten build and commits `gbemu.js` / `gbemu.wasm` to `web/public/` (or uploads them as build artifacts that Amplify pulls)
- Alternatively, include the Emscripten build as a step inside the Amplify `amplify.yml` build spec using the `emscripten/emsdk` Docker image

---

## Implementation Order

| # | Task | Estimated Effort |
|---|------|-----------------|
| 1 | APU skeleton — register map, MMU wiring, `tick()` stub | 1–2 days |
| 2 | CH1 + CH2 (square waves + envelopes) | 2–3 days |
| 3 | CH3 (wave) + CH4 (noise) + frame sequencer | 2–3 days |
| 4 | SDL2 audio output for native build | 1 day |
| 5 | Save state serialization/deserialization | 2 days |
| 6 | Emscripten WASM build + `web_bindings.cc` | 1–2 days |
| 7 | React frontend skeleton (canvas + rAF loop) | 1–2 days |
| 8 | ROM uploader + GamepadInput | 1 day |
| 9 | Web Audio integration | 1–2 days |
| 10 | Supabase project setup (buckets, schema, RLS) | 0.5 days |
| 11 | `useSaveStates` + `RomUploader` wired to Supabase | 1 day |
| 12 | Save state UI + thumbnails | 1 day |
| 13 | AWS Amplify + CloudFront deployment | 1 day |
| 14 | Optional: Express on Elastic Beanstalk for validation | 1 day |

**Total estimate: ~3–4 weeks** (working solo, part-time)

---

## Remaining Core TODOs

These are gaps in the current emulator core that need to be closed before most commercial ROMs will run correctly.

### MMU / Memory

| Location | TODO |
|----------|------|
| `mmu.cc:19` | Boot ROM — currently returns `0xFF`; implement or skip by setting `0xFF50 = 1` at startup |
| `mmu.cc:27,98` | VRAM read/write — currently no-ops; needs to route to `video_ram` in the Video class |
| `mmu.cc:67,146` | Interrupt Enable register (`0xFFFF`) and Interrupt Flag (`0xFF0F`) — reads/writes ignored |
| `mmu.cc` | IO register routing — most `0xFF00–0xFF7F` addresses (joypad, serial, timer, DMA) currently fall through to `unmapped_io_*`; each needs to be wired |

### CPU / Interrupts

| Location | TODO |
|----------|------|
| `cpu.cc:48` | HALT bug — when `IME=0` and a pending interrupt exists, the next byte after `HALT` is read twice; needed for some games |
| `cpu/opcodes.cc` | `STOP` currently aliases `HALT`; full STOP should also power down LCD and wait for button press |

### Cartridge

| Location | TODO |
|----------|------|
| `cartridge.h:50,69` | MBC1 + MBC3 ROM/RAM Mode Select (`0x6000–0x7FFF`) — bank switching mode not implemented |
| `cartridge.cc:52` | Bounds check on MBC ROM bank reads |
| *(missing)* | MBC5 — required by many commercial titles (Pokémon G/S/C, DK Country, etc.) |

### Video (PPU)

| Location | TODO |
|----------|------|
| `video.cc:190,257` | Tile fetch reads the full row of pixels per pixel — redundant work; replace with a proper pixel FIFO or per-row fetch |
| `video.cc:328` | Sprite-over-background priority not fully correct — needs BG colour 0 transparency check |
| `video.cc:357` | Duplicated tile-fetch logic between BG and window rendering; refactor into shared helper |
| `video.h:60–61` | CGB colour palettes and VRAM bank register (out of scope for DMG target, tracked for future) |

### Implementation Order (recommended)

1. Wire VRAM reads/writes through MMU → Video
2. Wire IO registers: joypad (`0xFF00`), timer (`0xFF04–0xFF07`), interrupt flag (`0xFF0F`), IE (`0xFFFF`)
3. Skip boot ROM (write `0x01` to `0xFF50` on startup) so games boot immediately
4. MBC1 ROM/RAM mode select
5. MBC5 support
6. Fix sprite priority (BG colour 0 transparency)
7. HALT bug

---

## Open Questions

- **MBC5 support** — Many commercial ROMs require MBC5; worth adding before the website goes live.
- **GBC (Color) support** — Color mode requires double-speed CPU, CGB palette registers, and VRAM banks. Large scope; keep DMG-only for now.
- **Authentication** — Supabase Auth (magic link or OAuth) can gate save states per user via RLS. Start with public anonymous access; flip the RLS policies to `auth.uid() = user_id` once login is added.
- **Legal** — Only allow ROM uploads of games the user owns. Add a terms-of-service acknowledgment on upload.
