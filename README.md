# Legaia Recomp

A static recompilation of **Legend of Legaia** (USA, `SCUS-94254`) for PC, with a
fix for the hard freeze that occurred during some battle attack transitions.

Built on [psxrecomp](https://github.com/mstan/psxrecomp) by Matthew Stanley and
[recomp-ui](https://github.com/RetroPortingToolKit/recomp-ui).

| | |
|---|---|
| Game | Legend of Legaia |
| Region | USA (`SCUS-94254`) |
| Players | 1 |
| Status | Playable; freeze fix verified in play |

## What this fork changes

The upstream project, scaffolded for this game, reproducibly **froze for 30-60
seconds** (then resumed, no crash) when certain Arts started, notably the
**Meta Gimard** attack transition and sometimes **Tornado Flame**.

### Root cause

The emulated CD-ROM controller (`psxrecomp/runtime/src/cdrom.c`) delayed the
CD interrupt line by 5000 cycles, but the guest-visible state did not honour that
delay. The interrupt flag register and the "response ready" status bit were
readable the instant a command was written. During the roughly 11 second XA
audio wait, the game's PsyQ libcd interrupt routine kept seeing a pending
interrupt, re-entered itself, and never returned to the main thread. The
game appeared frozen until the XA wait ended.

### Fix

The interrupt flag and the response-ready bit are now hidden from the guest until
the presentation delay has elapsed (`cd_irq_visible()`):

```c
static int cd_irq_visible(void) {
    return cdrom_irq_present_due == 0 || psx_cycle_count >= cdrom_irq_present_due;
}
// 0x1F801800 status:  response-ready bit only if cd_irq_visible()
// 0x1F801803 flag:    irq_flag only if cd_irq_visible()
```

This is the only change to the framework's runtime source. It is a small patch
that may be useful upstream for other PsyQ titles that stream XA audio.

**Tested:** Meta Gimard and Tornado Flame across multiple fights with no
freeze. Further regression testing of FMVs, streamed audio and loading
transitions is welcome; please open an issue if you see anything odd.

## Optional enhancements (mods)

Two experimental, **default-off** mods are included. Turn them on from the
launcher's **Mods** page. Both change presentation only; game logic, timing and
saves are unaffected.

- **Smooth Motion** blends the game's frames to present at 60 Hz, your display's
  refresh rate, or another rate you choose. It is a crossfade, not true in-between
  motion, so fast-moving objects can ghost slightly. OpenGL only; VSync is turned
  off while it is active; paused during movies.
- **Widescreen (Experimental)** presents 3D scenes (fields, towns, the world map,
  battles) at 16:9 or wider using a projection-squash method. Menus, the BIOS
  logos and movies stay 4:3. It is **not tuned for Legaia**: expect some
  edge pop-in or odd HUD/menu placement in places. Reports are welcome.

The launcher also offers the framework's built-in options, such as **Internal
Resolution** (up to 8K), image filtering, a CRT filter and enhanced audio.
Loading-speed mods are intentionally not included in public builds.

Widescreen needs these lines in `game.toml` (already set in this repo):

```toml
[widescreen]
native_wide = false
gte_game_mode = true
```

## Legal & Disclaimer

Legend of Legaia is a trademark of its respective owners (Sony Interactive
Entertainment / Contrail / Prokion). Legaia Recomp is an independent,
non-commercial, open-source project and is **not affiliated with, endorsed by,
or sponsored by** Sony or any of its subsidiaries or the game's developers.

- This repository contains **no game assets**: no disc images, ROM dumps, audio,
  video, textures, or retail PlayStation BIOS binaries.
- You **must supply your own legally purchased copy** of the game as a disc image.
  Disc images and saves are gitignored and must never be committed.
- Builds run with the bundled clean-room, MIT-licensed
  [OpenBIOS](https://github.com/grumpycoders/pcsx-redux/tree/main/src/mips/openbios).
  No retail BIOS is required or redistributed.
- `generated/` contains C code **machine-translated from the game's executable**
  by the recompiler. It is a derivative of the original program code, so it is
  published for preservation and interoperability purposes only. If you are a
  rights holder and have concerns, please open an issue and it will be addressed
  promptly.
- This project is **non-commercial**. Do not sell it or builds of it.

## License

The framework (`psxrecomp/`) is licensed under the
[PolyForm Noncommercial License 1.0.0](psxrecomp/LICENSE), copyright Matthew
Stanley. This fork keeps that license and its attribution, and all of it, including
the CD-ROM fix, is covered by the same noncommercial terms. See
`psxrecomp/THIRD_PARTY_ATTRIBUTION.md` for third-party components.

## Building

Requirements: CMake, Ninja, a C compiler (MSVC/clang on Windows, gcc/clang
on Linux), Python 3, and SDL2 (see `psxrecomp/docs/BUILDING.md`).

```bash
cmake -S . -B build-release -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --target psx-runtime
```

The recompiled game C in `generated/` is already committed, so no disc is needed
to build. To regenerate it from your own disc:

```bash
python3 psxrecomp/psxrecomp_cli.py generate \
  --config game.toml --project-root . --disc disc/<your>.cue
```

Place your disc image under `disc/` (for example `disc/Legend of Legaia.cue`)
before running. Config is in `game.toml` and `game_options.toml`.

## Debugging freezes

The runtime includes a freeze detector. If the game stalls it writes
`psx_freeze_dump_*.json` and `psx_freeze_heartbeat.json`, and
`psx_last_run_report.json` records why the last run ended. Attach these when
reporting an issue.

## Credits

- **psxrecomp**: Matthew Stanley ([mstan/psxrecomp](https://github.com/mstan/psxrecomp))
- **recomp-ui** and the Retro Porting ToolKit
- **OpenBIOS**: PCSX-Redux contributors (MIT)
- Original game: Contrail / Prokion, published by Sony Computer Entertainment
