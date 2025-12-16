#include "string.h"

#ifndef NULL    
#define NULL ((void*)0)
#endif

bool strcmp(const char* str1, const char* str2) {
    int i = 0;

    if (str1 == NULL || str2 == NULL) {
        return false;
    }

    while (true) {
        if (str1[i] != str2[i]) {
            return false;
        }

        if (str1[i] == '\0') {
            return true;
        }
        
        i++;
    }
}

int getLength(char* str) {
    int counter = 0;
    while (*str) {
        counter++;
        str++;
    }
    return counter;
}

void trim(char* str) {
    int length = getLength(str);
    int start = 0;
    int i = 0;

    while (str[start] == ' ') {
        start++;
    }

    while (*(str + length - 1) == ' ') {
        length--;
    }

    for (; i < length - start; i++) {
        str[i] = str[i + start];
    }

    str[length - start] = '\0';
}