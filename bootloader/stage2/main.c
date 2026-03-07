#include "stdio.h"
#include "string.h"
#include "stdint.h"
#include "commands.h"
#include "disk.h"
#include "fat.h"

int _cdecl cstart(uint16_t bootDrive) {
    char buffer[50];
    char displayPath[100];
    DirectoryEntry* currentDirEntry;
    DISK disk;
    clear();
    diskInitialize(&disk, bootDrive);
    fatInitialize(&disk, currentDirEntry);
    printf("Hello. Welcome to our OS. Please write help for instructions.\n\r");
    diskInitialize(&disk, bootDrive);
    while (true) {
        formatDisplayString(currentDirEntry->FileName, displayName);
        printf("%s>", displayName);
        readPrompt(buffer);
        handleCommand(buffer, &disk, currentDirEntry);
    }
}
