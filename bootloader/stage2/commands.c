#include "commands.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "stdpow.h"
#include "fat.h"
#include "disk.h"
#include "memory.h"

File far* open(DISK* disk, const char* path);

void ls_command(DISK* disk);

CommandEntry commands[] = {
    {"help", CMD_HELP, "all terminal commands"},
    {"clear", CMD_CLEAR, "clear screen"},
    {"poweroff", CMD_SHUTDOWN, "shutdown the system"},
    {"ls", CMD_VIEW_DIRS, "list directories"},
    {NULL, CMD_UNKNOWN, NULL}
};

CommandType getCommandType(char* input) {
    int i = 0;
    trim(input);
    while (commands[i].name != NULL) {
        if (strcmp(commands[i].name, input)) {
            return commands[i].type;
        }
        i++;
    }
    return CMD_UNKNOWN;
}

void handleCommand(CommandType type, DISK* disk) {
    int i;
    switch (type) {
        case CMD_HELP:
            printf("List of commands:\n\r");
            for (i = 0; commands[i].name != NULL; i++) {
                printf(" - %s (%s)\n\r", commands[i].name, commands[i].description);
            }
            break;
        case CMD_CLEAR:
            clear();
            break;
        case CMD_SHUTDOWN:
            shutdown();
            break;
        case CMD_VIEW_DIRS:
            ls_command(disk);
            break;
        case CMD_UNKNOWN:
            printf("Ya ustal. Net takoi commandi blin.\n\r");
            break;
    }
}

void ls_command(DISK* disk) {
    File far* file;
    DirectoryEntry entry;
    int i = 0;
    int j;

    file = open(disk, "/");

    while (readEntry(disk, file, &entry) && i++ < 5) {
        printf(" ");
        for (j = 0; j < 11; j++) {
            putc(entry.FileName[j]);
        }
        printf("\r\n");
    }
    close(file);
}