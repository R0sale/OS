#include "commands.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "stdpow.h"
#include "fat.h"
#include "disk.h"
#include "memory.h"

#define MAX_COMMAND_LENGTH 15
#define MAX_PARAMS 3
#define MAX_FILENAME_LENGTH 11

File far* open(DISK* disk, const char* path);

void ls_command(DISK* disk, DirectoryEntry* currentDirEntry);
void read_file(DISK* disk, char* path, DirectoryEntry* currentDirEntry); 

void change_directory(DISK* disk, const char* path, DirectoryEntry* currentDirEntry);
void create_full_path(char* path, DirectoryEntry* dirEntry, char* outBuffer);


CommandEntry commands[] = {
    {"help", CMD_HELP, "all terminal commands"},
    {"clear", CMD_CLEAR, "clear screen"},
    {"poweroff", CMD_SHUTDOWN, "shutdown the system"},
    {"ls", CMD_VIEW_DIRS, "list directories"},
    {"read", CMD_READ, "reads the file by path"},
    {"cd", CMD_CHANGE_DIR, "changes the directory"},
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

void handleCommand(char* buffer, DISK* disk, DirectoryEntry* dirEntry) {
    char* params[MAX_PARAMS];
    int i = 0;
    int j = 0;
    char* remaining = buffer;
    CommandType type;
    while (j < MAX_PARAMS) {
        char* tkn = get_token(&remaining, ' ');
        if (tkn == NULL) break;
        params[j] = tkn;
        j++;
    }

    type = getCommandType(params[0]);

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
            ls_command(disk, dirEntry);
            break;
        case CMD_READ:
            read_file(disk, params[1], dirEntry);
            break;
        case CMD_CHANGE_DIR:
            change_directory(disk, params[1], dirEntry);
            break;
        case CMD_UNKNOWN:
            printf("Ya ustal. Net takoi commandi blin.\n\r");
            break;
    }
}

void ls_command(DISK* disk, DirectoryEntry* currentDirEntry) {
    File far* file;
    DirectoryEntry entry;
    int i = 0;
    int j;
    char buffer[13];

    formatDisplayString(currentDirEntry->FileName, buffer);
    file = open(disk, buffer);

    while (readEntry(disk, file, &entry) && i++ < 5) {
        printf(" ");
        for (j = 0; j < 11; j++) {
            putc(entry.FileName[j]);
        }
        printf("\r\n");
    }
    close(file);
}

void read_file(DISK* disk, char* path, DirectoryEntry* currentDirEntry) {
    File far* file;
    char buffer[100];
    uint32_t readFromBuffer;
    uint16_t i;

    create_full_path(path, currentDirEntry, buffer);
    file = open(disk, buffer);

    if (readFromBuffer = read(disk, file, sizeof(buffer), buffer))
    {
        for (i = 0; i < readFromBuffer; i++)
        {
            if (buffer[i] == '\n')
            {
                printf("\r\n");
            }
            putc(buffer[i]);
        }
    }
    close(file);
}

void change_directory(DISK* disk, const char* path, DirectoryEntry* currentDirEntry)
{
    File far* file;
    DirectoryEntry* dirEntry;
    int i;
    char buffer[13];

    formatDisplayString(currentDirEntry->FileName, buffer);
    file = open(disk, buffer);

    if (file == NULL)
    {
        printf("Couldn't open the current directory.\r\n");
        return;
    }

    if (strcmp(path, ".."))
    {
        close(file);
        file = open(disk, "/");
        findFile(disk, file, "/", dirEntry);
        *currentDirEntry = *dirEntry;
        return;
    }

    if (!findFile(disk, file, path, dirEntry))
    {
        printf("Couldn't find file\n\r");
        close(file);
        return;
    }

    if (!(dirEntry->Attributes & 0x10)) // 0x10 -- directory attribute
    {
        printf("The path is not a directory.");
        return;
    }

    *currentDirEntry = *dirEntry;
}

void create_full_path(char* path, DirectoryEntry* dirEntry, char* outBuffer)
{
    int i;
    int fileNameLength;
    int pathLength = getLength(path);
    formatDisplayString(dirEntry->FileName, outBuffer);
    fileNameLength = getLength(outBuffer);
    outBuffer[fileNameLength] = '/';
    for (i = 0; i < pathLength; i++)
    {
        outBuffer[fileNameLength + i + 1] = path[i];
    } 

    outBuffer[fileNameLength + i + 1] = '\0';
}
