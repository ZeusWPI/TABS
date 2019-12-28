compile_and_run: compile_bootloader run_bootloader

compile_bootloader:
	rm -rf target/boot.bin
	nasm -f bin -o target/boot.bin bootloader/main.asm

run_bootloader:
	qemu-system-x86_64 -drive format=raw,file=target/boot.bin -drive format=raw,file=target/kernel.bin -monitor stdio
