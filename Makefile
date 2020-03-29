run: bin
	qemu-system-i386 -vga std -nodefaults -drive format=raw,file=target/boot.bin -monitor stdio -device e1000,netdev=u1,mac=aa:bb:cc:dd:ee:ff -object filter-dump,id=f1,netdev=u1,file=/tmp/dump.pcap -netdev tap,id=u1,ifname=tap0,script=no,downscript=no

run_kernelonly: compile_kernel
	qemu-system-i386 -kernel target/kernel/kernel.bin -monitor stdio -vga std -nodefaults -device e1000,netdev=u1,mac=aa:bb:cc:dd:ee:ff -object filter-dump,id=f1,netdev=u1,file=/tmp/dump.dat -netdev tap,id=u1,ifname=tap0,script=no,downscript=no

debug_kernel: compile_kernel
	qemu-system-i386 -s -S -kernel target/kernel/kernel.bin

clean:
	rm -r ./target/
	mkdir -p ./target/helpers
	mkdir ./target/bootloader
	mkdir ./target/kernel

bin: compile_bootloader compile_kernel compile_helpers
	./target/helpers/make_bin

compile_helpers:
	gcc -g -o ./target/helpers/make_bin ./helpers/make_bin.c

compile_bootloader:
	rm -rf target/bootloader/bootloader.bin
	nasm -f bin -o target/bootloader/bootloader.bin bootloader/main.asm

compile_kernel:
	nasm -felf32 kernel/wrapper.asm -o target/kernel/wrapper.o
	i686-elf-gcc -g -c kernel/kernel.c -o target/kernel/kernel.o -std=gnu99 -ffreestanding -Og -Wall -Wextra -mno-80387 -mgeneral-regs-only -mno-red-zone
	i686-elf-gcc -g -T kernel/linker.ld -o target/kernel/kernel.bin -ffreestanding -Og -nostdlib target/kernel/wrapper.o target/kernel/kernel.o -lgcc
