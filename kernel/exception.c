#ifndef EXCEPTION_C
#define EXCEPTION_C

#include "terminal.c"

typedef struct interrupt_frame_struct {
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} interrupt_frame;

#define EXCEPTION_HANDLER_NO_ERR(NAME, STR) \
__attribute__((interrupt)) void NAME (interrupt_frame* frame) { \
    outb(0x20, 0x20); \
    bsod(frame, STR, -1); \
}

#define EXCEPTION_HANDLER_ERR(NAME, STR) \
__attribute__((interrupt)) void NAME (interrupt_frame* frame, uint32_t err_code) { \
    outb(0x20, 0x20); \
    bsod(frame, STR, err_code); \
}

void bsod(interrupt_frame* frame, char* err_msg, int32_t err_code) {

    terminal_initialize();
    terminal_writestring("An exception occured: ");
    terminal_writestring(err_msg);
    if(err_code != -1) {
        terminal_writestring(" (error code = 0x");
        terminal_writeint(err_code, 16);
        terminal_writestring(")");
    }
    terminal_writestring("\nHere's what we know:\n");
    terminal_writestring("eip = 0x");
    terminal_writeint(frame->eip, 16);
    terminal_writestring("\ncs = 0x");
    terminal_writeint(frame->cs, 16);
    terminal_writestring("\neflags = 0b");
    terminal_writeint(frame->eflags, 2);


    for(;;) {
        asm("hlt");
    }
}

EXCEPTION_HANDLER_NO_ERR(divide_by_zero_handler, "Divide by zero")
EXCEPTION_HANDLER_NO_ERR(debug_handler, "debug")
EXCEPTION_HANDLER_NO_ERR(non_maskable_interrupt_handler, "Non maskable interrupt")
EXCEPTION_HANDLER_NO_ERR(breakpoint_handler, "Breakpoint")
EXCEPTION_HANDLER_NO_ERR(overflow_handler, "Overflow")
EXCEPTION_HANDLER_NO_ERR(bound_range_exceeded_handler, "Bound range exceeded")
EXCEPTION_HANDLER_NO_ERR(invalid_opcode_handler, "Invalid opcode")
EXCEPTION_HANDLER_NO_ERR(device_not_available_handler, "Device not available")
EXCEPTION_HANDLER_ERR(double_fault_handler, "Double fault")
EXCEPTION_HANDLER_NO_ERR(coprocessor_seg_overrun_handler, "Coprocessor seg overrun")
EXCEPTION_HANDLER_ERR(invalid_tss_handler, "Invalid tss")
EXCEPTION_HANDLER_ERR(segment_not_present_handler, "Segment not present")
EXCEPTION_HANDLER_ERR(stack_segment_fault_handler, "Stack segment fault")
EXCEPTION_HANDLER_ERR(general_protection_fault_handler, "General protection fault")
EXCEPTION_HANDLER_ERR(page_fault_handler, "Page fault")
EXCEPTION_HANDLER_NO_ERR(reserved_handler, "Reserved")
EXCEPTION_HANDLER_NO_ERR(x87_fp_exception_handler, "x87 floating point exception")
EXCEPTION_HANDLER_NO_ERR(alignment_check_handler, "Alignment check");
EXCEPTION_HANDLER_NO_ERR(machine_check_handler, "Machine check");
EXCEPTION_HANDLER_NO_ERR(simd_fp_exception_handler, "SIMD floating point exception");
EXCEPTION_HANDLER_NO_ERR(virtualization_exception_handler, "Virtualization exception");
EXCEPTION_HANDLER_NO_ERR(security_exception_handler, "Security exception");

#endif //EXCEPTION_C