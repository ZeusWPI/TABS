# TABS

Totaal Arbitrair BesturingsSysteem, Zeus maakt een besturingssysteem!

## How to use

To compile, you need:

- `i686-elf-gcc` for cross-compiling the kernel
- A system `gcc` for compiling the host-helpers
- `nasm` for compiling the bootloader
- `qemu-system-i386` for emulating (`qemu-system-x86_64` will probably work too, but that is not directly supported)

If you run `make bin`, it will generate `target/boot.bin`, this is a binary file layed out as described in [the docs](docs/bootloader.md). To burn it on a USB drive, simply `dd if=target/boot.bin of=/dev/sdb` and the drive is bootable and contains an SFS filesystem with the files in `filesystem/`.

## Bootloader

The bootloader is self-written, based on articles on [the OSDev wiki](https://wiki.osdev.org/). It's a single stage, ELF-loading bootloader in the most basic way possible.

More info in [the docs](./docs/bootloader.md)

## Kernel

The kernel is based on [the bare bones kernel from the OSDev wiki](https://wiki.osdev.org/Bare_Bones).

### Features

- [x] Terminal output (with newlines!)
- [x] _Very_ basic (and probably broken) memory management
- [x] Interrupt handling
- [x] Keyboard input
- [x] Exception handling
- [x] Minimal shell
- [ ] Filesystem interaction
  - [ ] Show files in directory
  - [ ] Read files in directory
  - [ ] Write files in directory
- [ ] Tests
- [ ] Running executables from filesystem
- [x] Better memory management
- [ ] Better shell

As a test, I've implemented day 1 of [advent of code](https://adventofcode.com/) on the [AoC branch](https://github.com/Robbe7730/RoBoot/tree/AoC).
