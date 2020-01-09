#ifndef INTERRUPTS_C
#define INTERRUPTS_C

#include <stdint.h>
#include <stddef.h>

#include "memory.c"
#include "terminal.c"
#include "inline_asm.c"
#include "drivers/keyboard/keyboard.c"

typedef struct idt_entry_struct {
   uint16_t offset_1; // offset bits 0..15
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t zero;      // unused, set to 0
   uint8_t type_attr; // type and attributes, see below
   uint16_t offset_2; // offset bits 16..31
} idt_entry;

struct interrupt_frame {
    uint32_t esp;
    uint32_t ss;
    uint32_t gs;
    uint32_t fs;
    uint32_t es;
    uint32_t ds;
    uint32_t ebp;
    uint32_t edi;
    uint32_t esi;
    uint32_t edx;
    uint32_t ecx;
    uint32_t ebx;
    uint32_t eax;
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
};

idt_entry IDT[256];

void interrupt_new_handler(int intnum, void (*handler)(struct interrupt_frame*)) {
    uint32_t handler_address = (uint32_t) handler;
    IDT[intnum].offset_1 = (uint16_t) (handler_address & 0xffff);
    IDT[intnum].selector = 0x08;
    IDT[intnum].zero = 0;
    IDT[intnum].type_attr = 0b10001110; // Active, privilege level 00, storage segment = 0; 32 bit interrupt gate
    IDT[intnum].offset_2 = (uint16_t) ((handler_address & 0xffff0000) >> 16);
}

void interrupt_init() {
    /* ICW1 - begin initialization */
	outb(0x20, 0x11);
    outb(0xA0, 0x11);

    /* ICW2 - remap offset address of IDT */
    outb(0x21, 0x20);
    outb(0xA1, 0x82);

    /* ICW3 - setup cascading */
    outb(0x21, 0x00);
    outb(0xA1, 0x00);

    /* ICW4 - environment info */
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

	/* mask interrupts */
	outb(0x21 , 0xff);
	outb(0xA1 , 0xff);

    interrupt_new_handler(0x21, keyboard_handler);

    uint16_t size = (sizeof(idt_entry) * 256);

	lidt(IDT, size);

    keyboard_init();
}

#endif //INTERRUPTS_C