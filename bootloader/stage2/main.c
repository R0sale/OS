#include "stdio.h"
#include "stdint.h"

int _cdecl cstart(uint16_t bootDrive) {
    puts("Hello from main!\n\r");
    return 0;
}
