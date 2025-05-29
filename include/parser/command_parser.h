#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include "command/command.h"

#define MAX_COMMANDS 10
#define MAX_ARGS_PER_COMMAND 10

// Convert a string to an array of Command structs
void convert_input_to_commands(char *input, int *count, Command **commands);

#endif
