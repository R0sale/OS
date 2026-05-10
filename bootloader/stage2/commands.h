#pragma once
#include "stdint.h"
#include "fat.h"
#include "disk.h"

typedef enum {
    CMD_UNKNOWN = 0,
    CMD_HELP,
    CMD_CLEAR,
    CMD_SHUTDOWN,
    CMD_VIEW_DIRS,
    CMD_READ,
    CMD_CHANGE_DIR,
    CMD_MAKE_DIR,
    CMD_CALC
} CommandType;

typedef struct {
    char* name;
    CommandType type;
    char* description;
} CommandEntry;

CommandType getCommandType(char* entry);

void handleCommand(char* buffer, DISK* disk, char* cwd);
