#include "stdio.h"
#include "x86.h"

void putc(char c) {
    x86_Video_Write(c, 0);
}

void puts(const char* str)
{
    while (*str) {
        putc(*str);
        str++;
    }
}

#define PRINTF_STATE_DEFAULT 0
#define PRINTF_STATE_PERCENT 1

void _cdecl printf(const char* fmt, ...) {
    int* argp = (int*)&fmt;
    int state = PRINTF_STATE_DEFAULT;

    argp++;
    while (*fmt) {
        switch (state) {
            case PRINTF_STATE_DEFAULT:
                switch (*fmt) {
                    case '%':
                        state = PRINTF_STATE_PERCENT;
                        break;
                    default:
                        putc(*fmt);
                        break;
                }
                break;
            case PRINTF_STATE_PERCENT:
                switch (*fmt) {
                    case '%':
                        putc('%');
                        break;
                    case 'c':
                        putc((char)*argp);
                        argp++;
                        break;
                    case 's':
                        puts((char*)*argp);
                        argp++;
                        break;
                    default:
                        break;
                }
                state = PRINTF_STATE_DEFAULT;
                break;
        }
        fmt++;
    }
    
}
