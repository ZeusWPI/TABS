#ifndef SHELL_C
#define SHELL_C

#define SHELL_CMD_BUFFER_SIZE 128

#include <stddef.h>

#include "terminal.c"
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

int ree(char* unused) {
    terminal_initialize();
    terminal_putchar('R');
    for (int i = 1; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        terminal_putchar('e');
    }
    return 0;
}

int run_command(char* buffer) {
    // TODO If I make these 2 global, it breaks...
    // TODO This is ugly, fix this 
    const char* shell_commands_strings[] = {
        "echo",
        "hello",
        "cls",
        "ree",
        NULL
    };

    int (*shell_commands_functions[]) (char*) = {
        echo,
        hello,
        cls,
        ree
    };

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