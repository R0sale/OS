#include "memory.h"

void* memcpy(void* dst, const void* src, uint16_t num) {
    const uint8_t* u8BufferSrc = (const uint8_t*)src;
    uint8_t* u8BufferDst = (uint8_t*)dst;
    uint16_t i;

    for (i = 0; i < num; i++) {
        *(u8BufferDst + i) = *(u8BufferSrc+ i);
    }

    return u8BufferDst;
}

void* memset(void* ptr, int value, uint16_t num) {
    uint8_t* u8Buffer = (uint8_t*)ptr;
    uint16_t i;

    for (i = 0; i < num; i++) {
        *(u8Buffer) = (uint8_t)value;
    }

    return u8Buffer;
}

int memcmp(const void* ptr1, const void* ptr2, uint16_t num) {
    const uint8_t* u8BufferPtr1 = (const uint8_t*)ptr1;
    const uint8_t* u8BufferPtr2 = (const uint8_t*)ptr2;
    uint16_t i;

    for (i = 0; i < num; i++) {
        if (u8BufferPtr1[i] != u8BufferPtr2[i]) {
            return 1;
        }
    }

    return 0;
}