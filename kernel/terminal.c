#ifndef TERMINAL_C
#define TERMINAL_C

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define VGA_WIDTH 80
#define VGA_HEIGHT 25
#define MAX_ENTRIES (VGA_HEIGHT*VGA_WIDTH)
#define TERMINAL_BUFFER_START 0xB8000
#define BLANK_CHAR ' '

/* Hardware text mode color constants. */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

typedef struct terminal_entry{
    enum vga_color bg_color;
    enum vga_color fg_color;
    char value;
} terminal_entry;

typedef struct terminal_state{
    terminal_entry entries[MAX_ENTRIES];
    int position;
    enum vga_color bg_color, fg_color;
} terminal_state;

/* Clear the terminal with only blank spaces */
void __terminal_clear(terminal_state* state){
    uint16_t* terminal_buffer = (uint16_t*) TERMINAL_BUFFER_START;
    uint16_t empty_color = VGA_COLOR_BLACK | VGA_COLOR_GREEN << 4;
    for(size_t y=0;y<VGA_HEIGHT;++y)
        for(size_t x=0;x<VGA_WIDTH;++x){
			size_t index = y * VGA_WIDTH + x;
            terminal_buffer[index] = (uint16_t) ((unsigned char)BLANK_CHAR) | (uint16_t) empty_color << 8;
        }
}

/* Initialise a state, i.e. put every char on blank and the default color everywhere */
void terminal_initialize_state(terminal_state* state){
    state->position=0;
    state->bg_color = VGA_COLOR_BLACK;
    state->fg_color = VGA_COLOR_WHITE;
    for(size_t y=0;y<VGA_HEIGHT;++y)
        for(size_t x=0;x<VGA_WIDTH;++x){
			size_t index = y * VGA_WIDTH + x;
            state->entries[index].value = BLANK_CHAR;
        }
    __terminal_clear(state);
}

/* Copy the state from one to the other */
void terminal_state_copy(terminal_state* from, terminal_state* to){
    to->position=from->position;
    to->bg_color = from->bg_color;
    to->fg_color = from->fg_color;
    for(size_t y=0;y<VGA_HEIGHT;++y)
        for(size_t x=0;x<VGA_WIDTH;++x){
			size_t index = y * VGA_WIDTH + x;
            to->entries[index].value = from->entries[index].value;
        }
}

/* This function will update the terminal buffer with the given state */
void terminal_update(terminal_state *state){
    __terminal_clear(state);
    uint16_t* terminal_buffer = (uint16_t*) TERMINAL_BUFFER_START;
    for(size_t y=0;y<VGA_HEIGHT;++y)
        for(size_t x=0;x<VGA_WIDTH;++x){
			size_t index = y * VGA_WIDTH + x;
            terminal_entry entry = state->entries[index];
            uint8_t color = entry.fg_color | entry.bg_color << 4;
            terminal_buffer[index] = (uint16_t) ((unsigned char)entry.value) | (uint16_t) color << 8;
        }
}

/* Update the background color */
void __terminal_set_bg_color(terminal_state* state, enum vga_color color){
    state->bg_color = color;
}

/* Update the foreground color */
void __terminal_set_fg_color(terminal_state* state, enum vga_color color){
    state->fg_color = color;
}

/* Update the color */
void terminal_set_color(terminal_state* state, enum vga_color bg_color, enum vga_color fg_color){
    __terminal_set_bg_color(state, bg_color);
    __terminal_set_fg_color(state, fg_color);
}

/* Write a char to the state */
void __terminal_put_char(terminal_state *state, char c){
    size_t pos = state->position;
    terminal_entry entry;
    /* If there is a new line, keep going until the end of the line (with blank space in black) */
    if(c == '\n'){
        entry.bg_color = VGA_COLOR_BLACK;
        entry.fg_color = VGA_COLOR_BLACK;
        entry.value = BLANK_CHAR;
        while(pos % VGA_WIDTH){
            state->entries[pos] = entry;
            ++pos;
        }
        state->position = (pos % MAX_ENTRIES);
    }
    else{
        entry.bg_color = state->bg_color;
        entry.fg_color = state->fg_color;
        entry.value = c;
        state->entries[pos] = entry;
        state->position = ((pos+1) % MAX_ENTRIES);
    }
}

/* Write a char to the state and run an update */
void terminal_write_char(terminal_state *state, const char c){
    __terminal_put_char(state, c);
    terminal_update(state);
}

/* Write a string to the state and run an update */
void terminal_write_str(terminal_state *state, const char* str){
	size_t len = 0;
    while (str[len]) {
        __terminal_put_char(state, str[len]);
        ++len;
    }
    terminal_update(state);
}

/* Clear the terminal en reset the state to the initial state */
void terminal_clear_state(terminal_state* state){
    __terminal_clear(state);
    terminal_initialize_state(state);
}

char* itoa(unsigned int value, char* result, int base) {
	// check that the base if valid
	if (base < 2 || base > 36) { *result = '\0'; return result; }

	char* ptr = result, *ptr1 = result, tmp_char;
	int tmp_value;

	do {
		tmp_value = value;
		value /= base;
		*ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
	} while ( value );

	*ptr-- = '\0';
	while(ptr1 < ptr) {
		tmp_char = *ptr;
		*ptr--= *ptr1;
		*ptr1++ = tmp_char;
	}
	return result;
}

void terminal_write_int(terminal_state* state,int number, int base) {
	char* result = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
	itoa(number, result, base);
	terminal_write_str(state, result);
}

#endif //TERMINAL_C
