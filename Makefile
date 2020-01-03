run: bin
	qemu-system-i386 -drive format=raw,file=target/boot.bin -monitor stdio

clean:
	rm -r ./target/
	mkdir -p ./target/helpers
	mkdir ./target/bootloader
	mkdir ./target/kernel

bin: compile_bootloader compile_kernel compile_helpers
	./target/helpers/make_bin

compile_helpers:
	gcc -o ./target/helpers/make_bin ./helpers/make_bin.c

compile_bootloader:
	rm -rf target/bootloader/bootloader.bin
	nasm -f bin -o target/bootloader/bootloader.bin bootloader/main.asm

compile_kernel:
	nasm -felf32 kernel/wrapper.asm -o target/kernel/wrapper.o
	i686-elf-gcc -c kernel/kernel.c -o target/kernel/kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
	i686-elf-gcc -T kernel/linker.ld -o target/kernel/kernel.bin -ffreestanding -O2 -nostdlib target/kernel/wrapper.o target/kernel/kernel.o -lgcc
