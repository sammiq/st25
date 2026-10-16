# Star Trek: 25th Anniversary (CD, DOS) - portable version

This branch (`portable-version`) turns the decompilation into portable C that builds for 32/64 bit hosts with SDL2. The matching decompilation of the DOS executable, and its build, are on `main`.

This repository does not include the original game files, you must provide them yourself.

## Building and running

```sh
cmake -B build/port && cmake --build build/port -j     # needs SDL2; -DPLATFORM_NULL=ON builds a headless host
build/port/startrek --data orig/CD/TREKCD              # or set STARTREK_DATA; also --scale N, --fullscreen, --square, --amiga-music DIR
```

The game data is the original CD's `TREKCD` directory (`DATA.DIR`, `DATA.RUN`, `DATA.001`, `VOC/`). Saved games and the settings file go to `$XDG_DATA_HOME/startrek` (default `~/.local/share/startrek`). Old DOS saves can't be loaded.

### Amiga music and sound effects

The Amiga version's music and sampled sound effects can replace the AdLib ones. Convert them with `tools/amiga_music.py` (songs to ProTracker modules, effects to WAV files), from an Amiga install (default `orig/AMIGA/StarTrek25thAnnivAGA/data/database`):

```sh
tools/amiga_music.py ~/.local/share/startrek/amiga path/to/StarTrek25thAnnivAGA/data/database
```

A folder called `amiga` in the saved-games folder is used automatically. To use a folder somewhere else, pass `--amiga-music DIR` or set `STARTREK_AMIGA_MUSIC=DIR`. Set `STARTREK_AMIGA_MUSIC=` (empty) to keep the AdLib sound. The game plays an Amiga song or effect wherever the Amiga version did. The CD's digitised effects and speech are kept. Anything the Amiga version has nothing for, such as the intro logo music, stays on the AdLib. One deliberate difference: ground music 33 plays song 20, which is the same tune, where the Amiga version plays effect 34 instead.

## How it is put together

- `src/` the game, as C. `include/compat.h` supplies the 16 bit types it was written with. `include/platform.h` is the only boundary to the host: VGA mode 13h video, keyboard, mouse, joystick, timer, files with DOS-style names, all implemented in `platform/` (SDL2, or a headless one for tests).
- `src/rooms/*.c` the scripts of the 53 rooms: the original `.RDF` files' machine code, translated to C. They run against the loaded `.RDF` image and call the engine through `ScriptService` (`include/rooms.h`).
- `audio/` the sound hardware, emulated: an AdLib/Sound Blaster FM chip (Nuked OPL3, LGPL-2.1, in `audio/nuked/`) driven by an XMIDI sequencer standing in for the Miles AIL music driver, and the Sound Blaster DAC with its DMA buffer. Optionally the Amiga music, played by micromod (BSD, in `audio/micromod/`, patched to stop at the song's end) in `audio/mod.c`, and the Amiga sound effects in `audio/amiga_sfx.c`. `platform/plat_audio_sdl.c` is the output.
- The original's assembly (fixed point maths, decompression, line and sprite drawing, the scaler, mixing, ...) is C now, each replacement checked against the original machine code under the Unicorn emulator: `tests/test_*.py` (needs `.venv` with `unicorn`, and the unpacked original EXE in `build/exe/`).

## Testing hooks

`STARTREK_SHOT=file.ppm STARTREK_SHOT_MS=n` writes the screen after n ms and quits; `STARTREK_SHOTS=ms,ms,..` writes several; `STARTREK_INPUT="ms:key:0x1C0D,ms:click:x:y,..."` scripts input; `STARTREK_ROOM=demon:1` starts the away team in that room; `STARTREK_LOAD=savegame.N` loads that save a few seconds into the bridge and `STARTREK_ORDER=orbit` (or a crew order number) then gives that order. `SDL_AUDIODRIVER=disk SDL_DISKAUDIOFILE=out.raw` records the sound (16 bit stereo, 48 kHz).

## Development tools and tests

Python 3.11+ with `pip install -r requirements.txt` (`unicorn`) in `.venv`, and the original game in `orig/CD/TREKCD` (not in git).

| | |
|---|---|
| `tools/unexepack.py packed.exe out.exe` | unpacks the original (EXEPACKed) `STARTREK.EXE`; the tests load `build/exe/STARTREK.EXE` |
| `tools/oracle.py` | runs original functions under Unicorn (symbols from `config/symbols.txt`, `config/units.txt`) |
| `tools/extract_data.py` | unpacks the game's `DATA.001` archive into a directory |
| `tools/ppm2png.py` | converts the test hooks' screenshots |
| `tools/dosbox-startrek.conf` | runs the original under DOSBox for comparison (`dosbox -conf tools/dosbox-startrek.conf` from the repo root) |

```sh
mkdir -p build/exe && python3 tools/unexepack.py orig/CD/TREKCD/STARTREK.EXE build/exe/STARTREK.EXE
for t in seg027 seg003 gfx qsort; do .venv/bin/python tests/test_$t.py; done
```

The one mismatch `test_gfx.py` reports is an intentional negative-length `HLine` case.
