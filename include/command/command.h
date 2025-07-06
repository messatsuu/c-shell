#ifndef COMMAND_H
#define COMMAND_H

#include <unistd.h>

// 1u == 1 as unsigned integer
#define CMD_FLAG_PIPE       (1u << 0)
#define CMD_FLAG_REDIRECT   (1u << 1)
#define CMD_FLAG_BACKGROUND (1u << 2) // TODO: implement

typedef struct {
    char *command; // The whole passed command
    char **arguments; // The command, separated by spaces
    char separator[3];
    unsigned int flags;
    int output_file_descriptor;
    unsigned int number_of_arguments;
} Command;

int execute_command(Command *command);

#endif
