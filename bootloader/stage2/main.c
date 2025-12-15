#include "stdio.h"
#include "stdint.h"

int _cdecl cstart(uint16_t bootDrive) {
    puts("Hello from main!\n\r");
    printf("%% %c %s %c", '0', "my long long string", '2');
    for(;;);
}
