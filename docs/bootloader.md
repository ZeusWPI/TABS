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
| 0x00000 | 0x7ffff | _free space_              |
| 0x80000 | 0x9ffff | EDBA, partially usable    |
| 0xa0000 | 0xfffff | unusable                  |

## Higher Memory

| start    | end      | use                       |
|----------|----------|---------------------------|
| 0x00000  | 0xfffff  | Low memory                |
| 0x100000 | 0x200000 | **Kernel space**          |

## Bootsector layout

(all addresses are offsets from 0x7C00)

| start  | end    | use               |
|--------|--------|-------------------|
| 0x0000 | ?      | Bootloader code   |
| ?      | 0x0193 | _padding_         |
| 0x0194 | 0x01bd | SFS header        |
| 0x01be | ?      | _padding_         |
| ?      | 0x01fd | data              |
| 0x01fe | 0x01ff | 0x55AA boot magic |

## Hard Drive layout

| start      | end        | use                                        |
|------------|------------|--------------------------------------------|
| 0x00000000 | 0x000001ff | Bootloader code (including SFS superblock) |
| 0x00000200 | 0x000081ff | SFS reserved area (kernel ELF file)        |
| 0x00008200 | ???        | SFS data area                              |
| ???        | ???        | SFS free area                              |
| ???        | 0x00ffffff | SFS index area                             |
| 0x01000000 | end        | Unused (Disk size is set to 16MiB)         |

## GDT

| Entry | Base        | Limit   | Flags  | Access     | Purpose      |
|-------|-------------|---------|--------|------------|--------------|
| 0x0   | 0x00000000  | 0x00000 | 0b0000 | 0b00000000 | Null Segment |
| 0x8   | 0x00000000  | 0xfffff | 0b1100 | 0b10011010 | Code Segment |
| 0x10  | 0x00000000  | 0xfffff | 0b1100 | 0b10010010 | Data Segment |

## Known issues

### Stack setup

As I don't quite get how segments work in real mode, there are most likely errors in the way I set up my stack. I'm going on a _it works on my machine_ approach currently.

### Kernel is not a file

I reserve 32KiB of SFS reserved area for the ELF file of the kernel. This is currently for "historic reasons" (a.k.a. I'm too lazy to load the filesystem in the bootloader). The kernel then handles the filesystem.

### Second ELF program header does not get loaded

This will most likely fix the issue of missing global strings.
