#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include "command/command.h"

#define MAX_COMMANDS 10
#define MAX_ARGS_PER_COMMAND 20

// Convert a string to an array of Command structs
void convert_input_to_commands(char *input, int *count, Command **commands);

void cleanup_command(Command *command);

#endif
