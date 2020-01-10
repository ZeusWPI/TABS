#ifndef SHELL_C
#define SHELL_C

#define SHELL_CMD_BUFFER_SIZE 128

#include <stddef.h>

#include "terminal.c"
#include "inline_asm.c"
#include "drivers/keyboard/keyboard.c"

char buffer[SHELL_CMD_BUFFER_SIZE];
int buffer_idx = 0;

int echo(char* input) {
    terminal_writestring(input);
    terminal_putchar('\n');
    return 0;
}

int hello(char* unused) {
    terminal_writestring("Hello, world!\n");
    return 0;
}

int cls(char* unused) {
    terminal_initialize();
    return 0;
}

int get_gdt(char* unused) {
    gdt_desc desc = {2,2};
    sgdt(&desc);
    terminal_writestring("limit = ");
    terminal_writeint(desc.limit, 10);
    terminal_writestring("\nbase = 0x");
    terminal_writeint(desc.base, 16);
    terminal_putchar('\n');

    gdt_entry* entries = (gdt_entry*) desc.base;
    int num_entries = (desc.limit+1) / 8;
    for (int entry_num = 0; entry_num < num_entries; entry_num++) {
        gdt_entry entry = entries[entry_num];
        uint32_t base = entry.base_lower | entry.base_middle << 16 | entry.base_higher << 24;
        uint32_t limit = entry.limit_lower | (entry.flags_limit_higher & 0x0f) << 16;
        uint8_t flags = (entry.flags_limit_higher >> 4);
        bool is_data = ((entry.access_byte & 0b00001000) >> 3) == 0;
        
        //terminal_writestring("\nEntry ");
        //terminal_writeint(entry_num, 10);
        terminal_writestring("base = 0x");
        terminal_writeint(base, 16);
        terminal_writestring("\nlimit = 0x");
        terminal_writeint(limit, 16);
        terminal_writestring("\nflags = 0b");
        terminal_writeint((entry.flags_limit_higher >> 4), 2);

        if ((flags & 0b1000) == 0) {
            terminal_writestring(" (byte granularity");
        } else {
            terminal_writestring(" (page granularity");
        }

        if ((flags & 0b0100) == 0) {
            terminal_writestring(", 16 bit)");
        } else {
            terminal_writestring(", 32 bit)");
        }

        terminal_writestring("\naccess = 0b");
        terminal_writeint(entry.access_byte, 2);
        terminal_writestring(" (ring ");
        terminal_writeint((entry.access_byte & 0b01100000) >> 5, 10);
        if ((entry.access_byte & 0b00010000) == 0) {
            terminal_writestring(", System");
        }
        if (is_data) {
            terminal_writestring(", Data");

            if((entry.access_byte & 0b00000100) == 0) {
                terminal_writestring(" (growing up, ");
            } else {
                terminal_writestring(" (growing down, ");
            }

            if((entry.access_byte & 0b00000010) == 0) {
                terminal_writestring("r--)");
            } else {
                terminal_writestring("rw-)");
            }
        } else {
            terminal_writestring(", Code");

            if((entry.access_byte & 0b00000100) == 0) {
                terminal_writestring(" (non-conforming, ");
            } else {
                terminal_writestring(" (conforming, ");
            }

            if((entry.access_byte & 0b00000010) == 0) {
                terminal_writestring("--x)");
            } else {
                terminal_writestring("r-x)");
            }
        }

        terminal_writestring(")\n");
    }
    return 0;
}

int ree(char* unused) {
    terminal_initialize();
    terminal_putchar('R');
    for (int i = 1; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        terminal_putchar('e');
    }
    return 0;
}

// TODO This is ugly, fix this 
const char* shell_commands_strings[] = {
    "echo",
    "hello",
    "cls",
    "ree",
    "getgdt",
    NULL
};

int (*shell_commands_functions[]) (char*) = {
    echo,
    hello,
    cls,
    ree,
    get_gdt
};

int run_command(char* buffer) {

    if(buffer[0] == 0) {
        return 0;
    }
    char command[SHELL_CMD_BUFFER_SIZE] = {0};
    int i = 0;
    while (buffer[i] != 0 && buffer[i] != ' ') {
        command[i] = buffer[i];
        i++;
    }

    int command_idx = 0;

    while(shell_commands_strings[command_idx] != NULL) {
        int check_idx = 0;
        while(command[check_idx] != 0 && shell_commands_strings[command_idx][check_idx] == command[check_idx]) {
            check_idx++;
        }
        if (command[check_idx] == 0 && shell_commands_strings[command_idx][check_idx] == 0) {
            return (shell_commands_functions[command_idx])(buffer + i + 1);
        }
        command_idx++;
    }
    return -1;
}

void shell_step() {
    char curr_char = getchar();

    if (curr_char == '\n') {
        terminal_putchar(curr_char);
        buffer[buffer_idx] = 0;
        int result = run_command(buffer);
        for (int i = 0; i < SHELL_CMD_BUFFER_SIZE; i++) {
            buffer[i] = 0;
        }
        buffer_idx = 0;

        if (result == -1) {
            terminal_writestring("No such command\n");
        }
    } else if (curr_char == 0x08) {
        if (buffer_idx != 0) {
            buffer_idx--;
            buffer[buffer_idx] = 0;
            terminal_putchar(curr_char);
        }
    } else {
        buffer[buffer_idx] = curr_char;
        buffer_idx++;
        terminal_putchar(curr_char);
    }
}

#endif //SHELL_C