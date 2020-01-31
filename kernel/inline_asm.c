#ifndef INLINE_ASM_C
#define INLINE_ASM_C

#include <stdint.h>

#include "memory.c"

typedef struct {
    uint16_t limit;
    uint32_t base;
}__attribute__((packed)) gdt_desc;

typedef struct {
    uint16_t limit_lower;
    uint16_t base_lower;
    uint8_t base_middle;
    uint8_t access_byte;
    uint8_t flags_limit_higher;
    uint8_t base_higher;
}__attribute__((packed)) gdt_entry;

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0"
                   : "=a"(ret)
                   : "Nd"(port) );
    return ret;
}

static uint16_t inw(uint16_t port) {
	uint16_t ret;
	asm volatile ("inw %1, %0" : "=a" (ret) : "dN" (port));
	return ret;
}

static void outw(uint16_t port, uint16_t val) {
	asm volatile ("outw %0, %1" : : "a" (val), "dN" (port) );
}

static uint32_t inl(uint16_t port) {
	uint32_t ret;
	asm volatile ("inl %%dx, %%eax" : "=a" (ret) : "dN" (port));
	return ret;
}

static void outl(uint16_t port, uint32_t val) {
	asm volatile ("outl %%eax, %%dx" : : "dN" (port), "a" (val));
}

static inline void lidt(void* base, uint16_t size)
{   // This function works in 32 and 64bit mode
    struct {
        uint16_t length;
        void*    base;
    } __attribute__((packed)) IDTR = { size, base };

    asm ( "lidt %0" : : "m"(IDTR) );  // let the compiler choose an addressing mode
}

/** issue a single request to CPUID. Fits 'intel features', for instance
 *  note that even if only "eax" and "edx" are of interest, other registers
 *  will be modified by the operation, so we need to tell the compiler about it.
 */
static inline void cpuid(int code, uint32_t *a, uint32_t *d) {
    asm volatile("cpuid":"=a"(*a),"=d"(*d):"a"(code):"ecx","ebx");
}

/** issue a complete request, storing general registers output as a string
 */
static inline int cpuid_string(int code, uint32_t where[4]) {
    asm volatile("cpuid":"=a"(*where),"=b"(*(where+1)),
                "=c"(*(where+2)),"=d"(*(where+3)):"a"(code));
    return (int)where[0];
}

static inline void sgdt(gdt_desc* ret) {
    asm volatile ("sgdt %0" : : "m"(*ret) : "memory");
}

static void * memcpy(void * restrict dest, const void * restrict src, long n) {
	asm volatile("cld; rep movsb"
	            : "=c"((int){0})
	            : "D"(dest), "S"(src), "c"(n)
	            : "flags", "memory");
	return dest;
}

static void * memset(void * dest, int c, long n) {
	asm volatile("cld; rep stosb"
	             : "=c"((int){0})
	             : "D"(dest), "a"(c), "c"(n)
	             : "flags", "memory");
	return dest;
}

#endif //INLINE ASM_C
