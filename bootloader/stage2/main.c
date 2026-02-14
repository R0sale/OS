#include "stdio.h"
#include "stdint.h"
#include "commands.h"
#include "disk.h"
#include "fat.h"

int _cdecl cstart(uint16_t bootDrive) {
    char buffer[50];
    DISK disk;
    clear();
    diskInitialize(&disk, bootDrive);
    fatInitialize(&disk);
    printf("Hello. Welcome to our OS. Please write help for instructions.\n\r");
    diskInitialize(&disk, bootDrive);
    while (true) {
        readPrompt(buffer);
        handleCommand(buffer, &disk);
    }
}
