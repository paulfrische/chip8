# chip8
## Build & Use
1. install meson
2. build
```bash
$ meson setup builddir
$ cd builddir
$ meson compile
```
3. profit (`chip8 <path to rom>`)

Example roms can be found [here](https://github.com/kripod/chip8-roms) (tested with `Breakout` and `Airplane`).

## TODO
- [ ] sound
- [ ] FIX: sometimes the emulator hangs with the breakout example (is it part of the game?)
