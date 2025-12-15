#include "stdio.h"
#include "stdint.h"

int _cdecl cstart(uint16_t bootDrive) {
    puts("Hello from main!\n\r");
    printf("Hello dear friend!!! %d   %s", -365, "my long long story");
    for(;;);
}
