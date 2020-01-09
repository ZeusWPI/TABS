#ifndef DRIVERS_KEYBOARD_KEYMAPS_DUMMY_AZERTY_C
#define DRIVERS_KEYBOARD_KEYMAPS_DUMMY_AZERTY_C

#include "../keycodes.c"

keycode get_keycode(unsigned char scancode) {
    switch (scancode)
    {
    case 0x10:
        return KEYCODE_A;
    case 0x30:
        return KEYCODE_B;
    case 0x2e:
        return KEYCODE_C;
    case 0x20:
        return KEYCODE_D;
    case 0x12:
        return KEYCODE_E;
    case 0x21:
        return KEYCODE_F;
    case 0x22:
        return KEYCODE_G;
    case 0x23:
        return KEYCODE_H;
    case 0x17:
        return KEYCODE_I;
    case 0x24:
        return KEYCODE_J;
    case 0x25:
        return KEYCODE_K;
    case 0x26:
        return KEYCODE_L;
    case 0x27:
        return KEYCODE_M;
    case 0x31:
        return KEYCODE_N;
    case 0x18:
        return KEYCODE_O;
    case 0x19:
        return KEYCODE_P;
    case 0x1e:
        return KEYCODE_Q;
    case 0x13:
        return KEYCODE_R;
    case 0x1f:
        return KEYCODE_S;
    case 0x14:
        return KEYCODE_T;
    case 0x16:
        return KEYCODE_U;
    case 0x2f:
        return KEYCODE_V;
    case 0x2c:
        return KEYCODE_W;
    case 0x2d:
        return KEYCODE_X;
    case 0x15:
        return KEYCODE_Y;
    case 0x11:
        return KEYCODE_Z;
    case 0x39:
        return KEYCODE_SPACE;
    case 0x2a:
        return KEYCODE_SHIFT_DOWN;
    case 0xaa:
        return KEYCODE_SHIFT_UP;
    case 0x36:
        return KEYCODE_SHIFT_DOWN;
    case 0xb6:
        return KEYCODE_SHIFT_UP;
    case 0x3a:
        return KEYCODE_CAPS_DOWN;
    case 0xba:
        return KEYCODE_CAPS_UP;
    default:
        return KEYCODE_UNKNOWN;
    }
}

#endif //DRIVERS_KEYBOARD_KEYMAPS_DUMMY_AZERTY_C