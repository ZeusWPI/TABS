# RoBoot

My attempt at making a bootloader and kernel.

## Bootloader

The bootloader is self-written, based on articles on [the OSDev wiki](https://wiki.osdev.org/). It's a single stage, ELF-loading bootloader in the most basic way possible.

More info in [the docs](./docs/bootloader.md)

## Kernel

The kernel is basically [the bare bones kernel from the OSDev wiki](https://wiki.osdev.org/Bare_Bones). I have implemented newlines, but otherwise it's still exactly the same...

As a more "functional" example, I've implemented day 1 of [advent of code](https://adventofcode.com/) on the [AoC branch](https://github.com/Robbe7730/RoBoot/tree/AoC).
