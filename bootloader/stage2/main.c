#include "stdio.h"
#include "stdint.h"
#include "commands.h"
#include "disk.h"

int _cdecl cstart(uint16_t bootDrive) {
    char buffer[50];
    DISK disk;
    CommandType type;
    clear();
    printf("Hello. Welcome to our OS. Please write help for instructions.\n\r");
    diskInitialize(&disk, bootDrive);
    while (true) {
        readPrompt(buffer);
        type = getCommandType(buffer);
        handleCommand(type);
    }
}
