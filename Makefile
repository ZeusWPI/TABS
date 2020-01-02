run_bootloader: compile_bootloader
	qemu-system-i386 -drive format=raw,file=target/boot.bin -monitor stdio

run_kernel: compile_kernel
	qemu-system-i386 -kernel target/kernel.bin

compile_bootloader: compile_kernel
	rm -rf target/bootloader.bin
	nasm -f bin -o target/bootloader.bin bootloader/main.asm
	cat target/bootloader.bin target/kernel.bin > target/boot.bin

compile_kernel:
	nasm -felf32 kernel/boot.asm -o target/boot.o
	i686-elf-gcc -c kernel/kernel.c -o target/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
	i686-elf-gcc -T kernel/linker.ld -o target/kernel.bin -ffreestanding -O2 -nostdlib target/boot.o target/kernel.o -lgcc
