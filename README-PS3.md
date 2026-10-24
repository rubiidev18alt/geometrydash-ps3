# Geometry Dash PS3 — native gameplay prototype

This is the first, deliberately small **native PS3 homebrew prototype**. It is
not the complete Geometry Dash port yet: it does not link the existing menus,
levels, or music. It provides a playable one-screen Geometry Dash-style run:
title prompt, auto-running cube, jump physics, block and spike hazards,
collision/retry, pause, and a finish line. The software renderer now uses the
repository's original Cube 1 layers (default green primary and cyan secondary,
with glow disabled), block, spike, background, and ground artwork. A small
standard-library PNG packer converts those images to an embedded RGBA bundle,
so the PS3 runtime needs no file lookup or PNG decoder. The prototype renders
to the PS3's RSX display buffers and uses the PSL1GHT controller API.

Target: PS3 homebrew using CFW/HEN and the **PS3DEV/PSL1GHT** toolchain. It is
not a PSP EBOOT, PS3 Linux application, or a port for the PS3's PSP-compatibility
mode.

## Build and test

Set up the PS3DEV toolchain and PSL1GHT as described by the
[PS3DEV toolchain project](https://github.com/ps3dev/ps3toolchain) and
[PSL1GHT](https://github.com/ps3dev/PSL1GHT). Make sure `PS3DEV`, `PSL1GHT`,
and the toolchain's `bin` directories are available in your environment.

From the repository root:

```sh
make -f Makefile.ps3 test       # host tests for gameplay, embedded art, and renderer
make -f Makefile.ps3 preview    # host-renderer reference screenshots (PNG)
make -f Makefile.ps3 prototype  # native PS3 executable (.elf and .self)
```

The PS3 build outputs are under `ps3/prototype/`. Open `gdps3proto.self` (or
the ELF, if preferred) in a PS3 homebrew-compatible emulator such as RPCS3.
The emulator needs controller input enabled. This prototype has only been
host-tested in this checkout: no PS3DEV/PSL1GHT toolchain or PS3 emulator is
installed here, so the native executable has not yet been built or emulator
verified. `make -f Makefile.ps3 preview` produces PNGs from the same software
renderer on the host for visual review; those are **not emulator captures**.

## Preview screenshots

These are host-rendered prototype previews, not RPCS3 captures:

- [Gameplay — approach](docs/images/ps3-prototype/gameplay_approach.png)
- [Gameplay — mid-jump](docs/images/ps3-prototype/gameplay_jump.png)
- [Gameplay — later run](docs/images/ps3-prototype/gameplay_run.png)
- [Title screen](docs/images/ps3-prototype/title.png)
- [Paused](docs/images/ps3-prototype/paused.png)
- [Crashed](docs/images/ps3-prototype/crashed.png)
- [Complete](docs/images/ps3-prototype/complete.png)

## Prototype controls

| Controller input | Action |
|---|---|
| Cross | Start the run; jump while playing; retry after a crash or finish |
| Start | Pause / resume |
| Circle | Retry while paused or crashed |

The cube auto-runs. Avoid the spikes and reach the end of the progress bar.
Jumping is edge-triggered, matching Geometry Dash's tap-to-jump behavior.

## What's next

The prototype lives in `ps3/prototype/` and is intentionally separate from the
existing 3DS/PSP targets. Its gameplay state (`game.c`), embedded graphics
loader, and software renderer (`render.c`) can be tested with a normal host
compiler. Next steps are to replace the sample obstacle course with real level
data, then incrementally bring over the shared game simulation, sound, and
menus behind PS3 platform services.

`make -f Makefile.ps3 assets` remains available to stage the repository's
resources at `ps3/out/gdps3/romfs/`, including the original `.t3x` files and
source PNG/`.t3s` art. The prototype's six selected images are packed directly
from `gfx/` and embedded in the native executable instead of loading from that
staging folder.
