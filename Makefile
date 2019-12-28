run_kernel: compile_kernel
	qemu-system-i386 -kernel target/kernel.bin

compile_kernel:
	nasm -felf32 kernel/boot.asm -o target/boot.o
	i686-elf-gcc -c kernel/kernel.c -o target/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
	i686-elf-gcc -T kernel/linker.ld -o target/kernel.bin -ffreestanding -O2 -nostdlib target/boot.o target/kernel.o -lgcc

compile_bootloader:
	rm -rf target/boot.bin
	nasm -f bin -o target/boot.bin bootloader/main.asm

run_bootloader: compile_bootloader
	qemu-system-x86_64 -drive format=raw,file=target/boot.bin -drive format=raw,file=target/kernel.bin -monitor stdio
