# RoBootloader

## Low Memory

| start   | end     | use                       |
|---------|---------|---------------------------|
| 0x00000 | 0x004ff | BIOS stuff, don't touch   |
| 0x00500 | 0x06bff | _free space_              |
| 0x06c00 | 0x07bff | **Bootloader stack**      |
| 0x07c00 | 0x07dff | Bootloader                |
| 0x07e00 | 0x07fff | _free space_              |
| 0x08000 | 0x0ffff | **ELF file** (64 sectors) |
| 0x10000 | 0x7ffff | **kernel space**          |
| 0x80000 | 0x9ffff | EDBA, partially usable    |
| 0xa0000 | 0xfffff | unusable                  |

## Drive layout

The first 512 bytes are the bootloader, right after that the 32-bit kernel ELF-file, this kernel can be 32KiB big. In the future, this "kernel" can be replaced by a second stage of the bootloader.

## Known issues

### Stack setup

As I don't quite get how segments work in real mode, there are most likely errors in the way I set up my stack. I'm going on a _it works on my machine_ approach currently.
