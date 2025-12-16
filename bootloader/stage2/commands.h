#pragma once

typedef enum {
    CMD_UNKNOWN = 0,
    CMD_HELP,
    CMD_CLEAR,
    CMD_SHUTDOWN
} CommandType;

typedef struct {
    char* name;
    CommandType type;
} CommandEntry;

CommandType getCommandType(char* entry);

void handleCommand(CommandType type);