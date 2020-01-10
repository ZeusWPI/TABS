#ifndef DRIVERS_KEYBOARD_C
#define DRIVERS_KEYBOARD_C

#include <stdbool.h>
#include <stddef.h>

#include "keycodes.c"
#include "../../util/fifo.c"
#include "keymaps/DUMMY_azerty.c"

#define KEY_QUEUE_EMPTY FIFO_QUEUE_EMPTY

bool keyboard_shift_state;
bool keyboard_caps_state;

fifo* key_queue = NULL;

int scancode_to_char(unsigned char scancode) {
    keycode keycode = get_keycode(scancode);

    if (keycode == KEYCODE_UNKNOWN) {
        return -1;
    } else if (keycode == KEYCODE_SHIFT_DOWN) {
        keyboard_shift_state = true;
        return -1;
    } else if (keycode == KEYCODE_SHIFT_UP) {
        keyboard_shift_state = false;
        return -1;
    } else if (keycode == KEYCODE_CAPS_UP) {
        keyboard_caps_state = !keyboard_caps_state;
        return -1;
    } else if (keycode == KEYCODE_CAPS_DOWN) {
        return -1;
    } else if (keycode >= KEYCODE_A && keycode <= KEYCODE_Z) {
        if (keyboard_shift_state ^ keyboard_caps_state) {
            return (char) keycode;               // Upper case
        } else {
            return ((char) keycode) + 32;        // Lower case
        }
    } else {
        return (char) keycode;
    }

}

void handle_scancode(unsigned char scancode) {
    int keycode = scancode_to_char(scancode);
    if (keycode == -1) {
        return;
    } else {
        fifo_enqueue(key_queue, (char) keycode);
    }
}

__attribute__((interrupt)) void keyboard_handler(struct interrupt_frame* frame) {
    unsigned char scan_code = inb(0x60);
    outb(0x20, 0x20);

    handle_scancode(scan_code);
}

int getchar_nonblocking() {
    return fifo_dequeue(key_queue);
}

char getchar() {
    int curr = KEY_QUEUE_EMPTY;
    while ((curr = getchar_nonblocking()) == KEY_QUEUE_EMPTY) {
        asm("hlt"); // And don't catch fire
    }
    return (char) curr;
}

void keyboard_init() {
    keyboard_shift_state = false;
    keyboard_caps_state = false;
    key_queue = fifo_new();

    uint8_t mask = inb(0x21);

    mask = mask & ~(1 << 1);

	outb(0x21 , mask);
}

#endif //DRIVERS_KEYBOARD_C