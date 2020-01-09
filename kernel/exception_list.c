typedef enum exception_enum {
    DIVIDE_BY_ZERO              = 0,
    DEBUG                       = 1,
    NON_MASKABLE_INTERRUPT      = 2,
    BREAKPOINT                  = 3,
    OVERFLOW                    = 4,
    BOUND_RANGE_EXCEEDED        = 5,
    INVALID_OPCODE              = 6,
    DEVICE_NOT_AVAILABLE        = 7,
    DOUBLE_FAULT                = 8, // Error code = 0
    COPROCESSOR_SEG_OVERRUN     = 9,
    INVALID_TSS                 = 10, // Error code = (invalid selector index)
    SEGMENT_NOT_PRESENT         = 11, // Error code = (segment selector index)
    STACK_SEGMENT_FAULT         = 12, // Error code = (segment selector index) if invalid ssi, otherwise 0
    GENERAL_PROTECTION_FAULT    = 13, // Error code = (segment selector index) if invalid ssi, otherwise 0
    PAGE_FAULT                  = 14, // Error code = bitmap, see https://wiki.osdev.org/Exceptions#Page_Fault
    RESERVED_15                 = 15,
    X87_FP_EXCEPTION            = 16,
    ALIGNMENT_CHECK             = 17,
    MACHINE_CHECK               = 18,
    SIMD_FP_EXCEPTION           = 19,
    VIRTUALIZATION_EXCEPTION    = 20,
    RESERVED_21                 = 21,
    RESERVED_22                 = 22,
    RESERVED_23                 = 23,
    RESERVED_24                 = 24,
    RESERVED_25                 = 25,
    RESERVED_26                 = 26,
    RESERVED_27                 = 27,
    RESERVED_28                 = 28,
    RESERVED_29                 = 29,
    SECURITY_EXCEPTION          = 30,
    RESERVED_31                 = 31
} exception;