#include "memory.h"

void far* memcpy(void far* dst, const void far* src, uint16_t num) {
    const uint8_t far* u8BufferSrc = (const uint8_t far*)src;
    uint8_t far* u8BufferDst = (uint8_t far*)dst;
    uint16_t i;

    for (i = 0; i < num; i++) {
        *(u8BufferDst + i) = *(u8BufferSrc+ i);
    }

    return u8BufferDst;
}

void far* memset(void far* ptr, int value, uint16_t num) {
    uint8_t far* u8Buffer = (uint8_t far*)ptr;
    uint16_t i;

    for (i = 0; i < num; i++) {
        *(u8Buffer) = (uint8_t)value;
    }

    return u8Buffer;
}

int memcmp(const void far* ptr1, const void far* ptr2, uint16_t num) {
    const uint8_t far* u8BufferPtr1 = (const uint8_t far*)ptr1;
    const uint8_t far* u8BufferPtr2 = (const uint8_t far*)ptr2;
    uint16_t i;

    for (i = 0; i < num; i++) {
        if (u8BufferPtr1[i] != u8BufferPtr2[i]) {
            return 1;
        }
    }

    return 0;
}