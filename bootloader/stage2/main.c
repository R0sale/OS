#include "stdio.h"
#include "stdint.h"
#include "commands.h"

int _cdecl cstart(uint16_t bootDrive) {
    char buffer[50];
    CommandType type;
    clear();
    printf("Hello. Welcome to our OS. Please write help for instructions.\n\r");
    while (true) {
        readPrompt(buffer);
        type = getCommandType(buffer);
        handleCommand(type);
    }
}
