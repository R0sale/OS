#include "commands.h"
#include "string.h"
#include "stdio.h"
#include "stdint.h"
#include "stdpow.h"
#include "fat.h"
#include "disk.h"
#include "memory.h"
#include "solution.h"

#define MAX_COMMAND_LENGTH 15
#define MAX_PARAMS 3
#define MAX_FILENAME_LENGTH 11

File far* open(DISK* disk, const char* path);

void ls_command(DISK* disk, char* cwd);
void read_file(DISK* disk, char* path, char* cwd); 

void change_directory(DISK* disk, const char* path, char* cwd);
void create_full_path(char* path, char* cwd, char* outBuffer);
void make_directory(DISK* disk, const char* dirName, char* cwd);


CommandEntry commands[] = {
    {"help", CMD_HELP, "all terminal commands"},
    {"clear", CMD_CLEAR, "clear screen"},
    {"poweroff", CMD_SHUTDOWN, "shutdown the system"},
    {"ls", CMD_VIEW_DIRS, "list directories"},
    {"read", CMD_READ, "reads the file by path"},
    {"cd", CMD_CHANGE_DIR, "changes the directory"},
    {"mkdir", CMD_MAKE_DIR, "creates the directory"},
    {"calc", CMD_CALC, "calculates the method of finite elements"},
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

void handleCommand(char* buffer, DISK* disk, char* cwd) {
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
            ls_command(disk, cwd);
            break;
        case CMD_READ:
            read_file(disk, params[1], cwd);
            break;
        case CMD_CHANGE_DIR:
            change_directory(disk, params[1], cwd);
            break;
        case CMD_MAKE_DIR:
            make_directory(disk, params[1], cwd);
            break;
        case CMD_CALC:
            calc();
            break;
        case CMD_UNKNOWN:
            printf("Ya ustal. Net takoi commandi blin.\n\r");
            break;
    }
}

void ls_command(DISK* disk, char* cwd) {
    File far* file;
    DirectoryEntry entry;
    int i = 0;
    int j;

    file = open(disk, cwd);
    if (file == NULL)
    {
        printf("File %s not found\n\r", cwd);
        return;
    }

    while (readEntry(disk, file, &entry) && i++ < 5) {
        printf(" ");
        for (j = 0; j < 11; j++) {
            putc(entry.FileName[j]);
        }
        printf("\r\n");
    }
    close(file);
}

void read_file(DISK* disk, char* path, char* cwd) {
    File far* file;
    char buffer[100];
    uint32_t readFromBuffer;
    uint16_t i;

    create_full_path(path, cwd, buffer);
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

void change_directory(DISK* disk, const char* path, char* cwd)
{
    File far* file;
    DirectoryEntry dirEntry;
    int i;

    file = open(disk, cwd);

    if (file == NULL)
    {
        printf("Couldn't open the current directory.\r\n");
        return;
    }

    if (strcmp(path, ".."))
    {
        close(file);
        file = open(disk, "/");
        findFile(disk, file, "/", &dirEntry);
        cwd[0] = '\0';
        close(file);
        return;
    }

    if (!findFile(disk, file, path, &dirEntry))
    {
        printf("Couldn't find file\n\r");
        close(file);
        return;
    }

    if (!(dirEntry.Attributes & 0x10)) // 0x10 -- directory attribute
    {
        printf("The path is not a directory.");
        close(file);
        return;
    }

    update_cwd(cwd, &dirEntry);
    close(file);
}

void create_full_path(char* path, char* cwd, char* outBuffer)
{
    int cwdLen = getLength(cwd);
    int pathLen = getLength(path);
    int outPos = cwdLen;

    memcpy(outBuffer, cwd, cwdLen);

    if (cwdLen > 0 && outBuffer[cwdLen - 1] != '/')
    {
        outBuffer[outPos] = '/';
        outPos++;
    }

    memcpy(outBuffer + outPos, path, pathLen);
    outPos += pathLen;

    outBuffer[outPos] = '\0';
}

void make_directory(DISK* disk, const char* dirName, char* cwd)
{
    if (makeDirectory(disk, cwd, dirName))
    {
        printf("Directory was successfully writen.\n\r");
    }
    else
    {
        printf("Couldn't write directory\n\r");
    }
}

void update_cwd(char* cwd, DirectoryEntry* dirEntry)
{
    char buffer[13];
    int cwdLen = getLength(cwd);
    int bufferLen;
    formatDisplayString(dirEntry->FileName, buffer);

    bufferLen = getLength(buffer);

    if (cwdLen > 0 && cwd[cwdLen - 1] != '/')
    {
        cwd[cwdLen] = '/';
        cwd[cwdLen + 1] = '\0';
        cwdLen++;
    }

    memcpy(cwd + cwdLen, buffer, bufferLen);
    cwd[cwdLen + bufferLen] = '\0';
}

void calc(void)
{
    __asm
    {
        finit
    }
    solve();
}
