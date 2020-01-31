#ifndef INTERRUPTS_C
#define INTERRUPTS_C

#include <stdint.h>
#include <stddef.h>

#include "memory.c"
#include "terminal.c"
#include "inline_asm.c"
#include "exception.c"
#include "exception_list.c"
#include "drivers/keyboard/keyboard.c"

typedef struct idt_entry_struct {
   uint16_t offset_1; // offset bits 0..15
   uint16_t selector; // a code segment selector in GDT or LDT
   uint8_t zero;      // unused, set to 0
   uint8_t type_attr; // type and attributes, see below
   uint16_t offset_2; // offset bits 16..31
} idt_entry;

idt_entry IDT[256];

void interrupt_new_handler(int intnum, void (*handler)(interrupt_frame*)) {
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
    outb(0xA1, 0x28);

    /* ICW3 - setup cascading */
    outb(0x21, 0x00);
    outb(0xA1, 0x00);

    /* ICW4 - environment info */
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    /* mask interrupts */
    outb(0x21 , 0xff);
    outb(0xA1 , 0xff);

    // Exceptions
    interrupt_new_handler(DIVIDE_BY_ZERO,           divide_by_zero_handler);
    interrupt_new_handler(DEBUG,                    debug_handler);
    interrupt_new_handler(NON_MASKABLE_INTERRUPT,   non_maskable_interrupt_handler);
    interrupt_new_handler(BREAKPOINT,               breakpoint_handler);
    interrupt_new_handler(OVERFLOW,                 overflow_handler);
    interrupt_new_handler(BOUND_RANGE_EXCEEDED,     bound_range_exceeded_handler);
    interrupt_new_handler(INVALID_OPCODE,           invalid_opcode_handler);
    interrupt_new_handler(DEVICE_NOT_AVAILABLE,     device_not_available_handler);
    interrupt_new_handler(DOUBLE_FAULT,             double_fault_handler);
    interrupt_new_handler(COPROCESSOR_SEG_OVERRUN,  coprocessor_seg_overrun_handler);
    interrupt_new_handler(INVALID_TSS,              invalid_tss_handler);
    interrupt_new_handler(SEGMENT_NOT_PRESENT,      segment_not_present_handler);
    interrupt_new_handler(STACK_SEGMENT_FAULT,      stack_segment_fault_handler);
    interrupt_new_handler(GENERAL_PROTECTION_FAULT, general_protection_fault_handler);
    interrupt_new_handler(PAGE_FAULT,               page_fault_handler);
    interrupt_new_handler(RESERVED_15,              reserved_handler);
    interrupt_new_handler(X87_FP_EXCEPTION,         x87_fp_exception_handler);
    interrupt_new_handler(ALIGNMENT_CHECK,          alignment_check_handler);
    interrupt_new_handler(MACHINE_CHECK,            machine_check_handler);
    interrupt_new_handler(SIMD_FP_EXCEPTION,        simd_fp_exception_handler);
    interrupt_new_handler(VIRTUALIZATION_EXCEPTION, virtualization_exception_handler);
    interrupt_new_handler(RESERVED_21,              reserved_handler);
    interrupt_new_handler(RESERVED_22,              reserved_handler);
    interrupt_new_handler(RESERVED_23,              reserved_handler);
    interrupt_new_handler(RESERVED_24,              reserved_handler);
    interrupt_new_handler(RESERVED_25,              reserved_handler);
    interrupt_new_handler(RESERVED_26,              reserved_handler);
    interrupt_new_handler(RESERVED_27,              reserved_handler);
    interrupt_new_handler(RESERVED_28,              reserved_handler);
    interrupt_new_handler(RESERVED_29,              reserved_handler);
    interrupt_new_handler(SECURITY_EXCEPTION,       security_exception_handler);
    interrupt_new_handler(RESERVED_31,              reserved_handler);

    interrupt_new_handler(0x21, keyboard_handler);

    uint16_t size = (sizeof(idt_entry) * 256);

    lidt(IDT, size);

    keyboard_init();

}

#endif //INTERRUPTS_C
