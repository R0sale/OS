#include "stdio.h"
#include "stdint.h"

int _cdecl cstart(uint16_t bootDrive) {
    clear();
    printf("Hello. Welcome to our OS. Please write help for instructions.");
    for(;;);
}
