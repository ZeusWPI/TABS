/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This kernel needs to be compiled with a ix86-elf compiler"
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "terminal.c"
#include "memory.c"
#include "interrupts.c"
#include "shell.c"

static inline bool are_interrupts_enabled() {
    unsigned long flags;
    asm volatile ( "pushf\n\t"
                   "pop %0"
                   : "=g"(flags) );
    return flags & (1 << 9);
}

void kernel_main(void) 
{
	/* Initialize terminal interface */
	terminal_initialize();

	terminal_putchar('H');
	terminal_putchar('e');
	terminal_putchar('l');
	terminal_putchar('l');
	terminal_putchar('o');
 
    terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    terminal_writestring(" kernel");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring(" World!\n");
	terminal_writestring("Newlines!\n");

	char* memory_str = alloc(sizeof(char) * 7);
	for (int i = 0; i < 6; i++) {
		memory_str[i] = "Memory"[i];
	}
	memory_str[6] = 0;

	char* management_str = alloc(sizeof(char) * 13);
	for (int i = 0; i < 13; i++) {
		management_str[i] = " management!\n"[i];
	}
	management_str[13] = 0;

	terminal_writestring(memory_str);
	terminal_writestring(management_str);

	terminal_writestring((are_interrupts_enabled())? "Interrupts!\n": "No interrupts :(\n");

	interrupt_init();

	for(;;) {
		shell_step();
	}
}