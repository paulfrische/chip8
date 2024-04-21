# chip8
![image](https://github.com/paulfrische/chip8/assets/61984114/e5b727ab-5e6b-4063-a1c4-507eb1cdea61)

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

## Warnings
- ⚠️  I don't know if this thing works on anything else then my CPU because of bit ordering and endianness (at least this shouldn't be an issue).
- ⚠️  I have basically no experience with emulators so this thing could be/is pretty buggy

## Resources to build your own
- https://tobiasvl.github.io/blog/write-a-chip-8-emulator/
- https://chip8.gulrak.net/
- https://github.com/Timendus/chip8-test-suite

## TODO
- [ ] sound
- [ ] make better debugging UI via raygui
- [ ] FIX: sometimes the emulator hangs with the breakout example (is it part of the game?)
