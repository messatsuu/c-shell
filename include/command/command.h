#ifndef COMMAND_H
#define COMMAND_H

#include <unistd.h>

// 1u == 1 as unsigned integer
#define CMD_FLAG_PIPE       (1u << 0)
#define CMD_FLAG_REDIRECT   (1u << 1)
#define CMD_FLAG_BACKGROUND (1u << 2) // TODO: implement

// TODO: put struct in command.c
typedef struct {
    char *command;
    char separator[3];
    unsigned int flags;
    int output_file_descriptor;
} Command;

int execute_command(Command *command);

#endif
