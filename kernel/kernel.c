#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
 
#include "terminal.c"

/* Check if the compiler thinks you are targeting the wrong operating system. */
#if defined(__linux__)
#error "You are not using a cross-compiler, you will most certainly run into trouble"
#endif
 
/* This tutorial will only work for the 32-bit ix86 targets. */
#if !defined(__i386__)
#error "This tutorial needs to be compiled with a ix86-elf compiler"
#endif

 
void kernel_main(void) 
{
	/* Initialize terminal interface */
	terminal_initialize();

	terminal_putchar('H');
	terminal_putchar('e');
	terminal_putchar('l');
	terminal_putchar('l');
	terminal_putchar('o');
 
	/* Newline support is left as an exercise. */
    terminal_setcolor(vga_entry_color(VGA_COLOR_GREEN, VGA_COLOR_BLACK));
    terminal_writestring(" kernel");
    terminal_setcolor(vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK));
    terminal_writestring(" World!\n");
	terminal_writestring("Newlines!");
}