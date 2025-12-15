#include "stdio.h"
#include "x86.h"

typedef unsigned char bool;

#define true 1
#define false 0

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

void clear(void)
{
    x86_Video_Clear();
}

char readKey(void) {
    char c = x86_Video_Read_Char();
    putc(c);
    return c;
}

void readPrompt(char* buffer)
{
    char c;
    int i = 0;
    while ((c = readKey()) != '\r') {
        if (i >= 50) {
            buffer = NULL;
            return;
        }
        buffer[i++] = c;
    }
    putc('\n');
    buffer[i] = '\0';
}


#define PRINTF_STATE_DEFAULT 0
#define PRINTF_STATE_PERCENT 1

void printf_number(int* argp, bool sign);

void _cdecl printf(const char* fmt, ...) {
    int* argp = (int*)&fmt;
    int state = PRINTF_STATE_DEFAULT;
    bool sign = false;

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
                    case 'd':
                        sign = true;
                        printf_number(argp, sign);
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

const char chars[] = "0123456789";

void printf_number(int* argp, bool sign) {
    int number = *argp;
    bool negativeNumber = false;
    char buffer[32];
    int pos = 0;

    if (sign) {
        if (number < 0) {
            number = -1 * number;
            negativeNumber = true;
        }
    }

    for (; number > 0; number /= 10) {
        buffer[pos++] = chars[number % 10];
    }

    if (negativeNumber) {
        buffer[pos] = '-';
    } 
    else {
        pos--;
    }

    while (pos >= 0) {
        putc(buffer[pos--]);
    }
}
