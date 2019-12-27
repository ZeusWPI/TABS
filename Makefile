compile_and_run: compile run

compile:
	rm -rf boot.bin
	nasm -f bin -o boot.bin main.asm

run:
	qemu-system-x86_64 -drive format=raw,file=boot.bin -drive format=raw,file=drive.bin -monitor stdio
