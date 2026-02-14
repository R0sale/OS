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

char* get_token(char** current_pos, char separator) {
    char* end;
    char* start = *current_pos;
    
    // 1. Skip leading separators (like multiple spaces)
    while (*start && *start == separator) {
        start++;
    }

    if (*start == '\0') return NULL; // End of string reached

    // 2. Find the end of the word
    end = start;
    while (*end && *end != separator) {
        end++;
    }

    // 3. If we found a separator, kill it with a null terminator
    if (*end != '\0') {
        *end = '\0';
        *current_pos = end + 1; // Move the position to the next word
    } else {
        *current_pos = end; // No more tokens left
    }

    return start;
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

const char* strchr(const char* str, char chr) {
    if (str == NULL) {
        return NULL;
    }

    while (*str) {
        if (*str == chr) {
            return str;
        }
        str++;
    }

    return NULL;
}