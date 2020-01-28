#ifndef TABS_PRINTER_C
#define TABS_PRINTER_C

#include <stdarg.h>

#include "../terminal.c"

/**
 * fmt is a string with specifiers where variables can be inserted.
 *
 * ex: ("Hello %s", "world") -> "Hello world"
 *
 * Specifiers are
 *   - %c: character
 *   - %d: digit (integer)
 *   - %p: pointer (void*)
 * @param fmt Formatter string
 * @param ... Variable amount of arguments to be inserted
 */
void print(const char *fmt, ...) {
    const char *p;
    va_list argp;
    int i;
    char *s;
    char fmtbuf[256];

    va_start(argp, fmt);

    for (p = fmt; *p != '\0'; p++)
        if (*p != '%') {
            terminal_putchar(*p);
        } else {
            switch (*++p) {
                case 'c':
                    i = va_arg(argp, int);

                    terminal_putchar((char) i);
                    break;
                case 'd':
                    i = va_arg(argp, int);
                    s = itoa(i, fmtbuf, 10);
                    terminal_writestring(s);
                    break;
                case 's':
                    s = va_arg(argp, char *);
                    terminal_writestring(s);
                    break;
                case 'x':
                    i = va_arg(argp, int);
                    s = itoa(i, fmtbuf, 16);
                    terminal_writestring(s);
                    break;
                case '%':
                    terminal_putchar('%');
                    break;
            }
        }

    va_end(argp);
}

#endif //TABS_PRINTER_C