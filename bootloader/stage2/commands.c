#include "commands.h"
#include "string.h"
#include "stdio.h"
#include "stdpow.h"

CommandEntry commands[] = {
    {"help", CMD_HELP},
    {"clear", CMD_CLEAR},
    {"poweroff", CMD_SHUTDOWN},
    {NULL, CMD_UNKNOWN}
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

void handleCommand(CommandType type) {
    switch (type) {
        case CMD_HELP:
            printf("List of commands: \n\r help - all terminal commands\n\r clear - clear screen\n\r poweroff - shutdown the system\n\r");
            break;
        case CMD_CLEAR:
            clear();
            break;
        case CMD_SHUTDOWN:
            shutdown();
            break;
        case CMD_UNKNOWN:
            printf("Ya ustal. Net takoi commandi blin.\n\r");
            break;
    }
}