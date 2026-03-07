#include "stdio.h"
#include "string.h"
#include "stdint.h"
#include "commands.h"
#include "disk.h"
#include "fat.h"

int _cdecl cstart(uint16_t bootDrive) {
    char buffer[50];
    char cwd[100] = "";
    DISK disk;
    clear();
    diskInitialize(&disk, bootDrive);
    fatInitialize(&disk);
    printf("Hello. Welcome to our OS. Please write help for instructions.\n\r");
    diskInitialize(&disk, bootDrive);
    while (true) {
        printf("%s>", cwd);
        readPrompt(buffer);
        handleCommand(buffer, &disk, cwd);
    }
}
