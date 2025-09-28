#ifndef PARSER_H
#define PARSER_H

#include "command/command.h"
#define MAX_COMMANDS 10

int convert_env_var(char **pointer, char **buffer, unsigned int *buffer_size, unsigned long *index);

// Conversions that mutate the original input that gets persisted in the history. Takes ownership of `input`.
void mutate_original_input(char **input);

// Convert raw input from the user
void convert_argv(char **argv);

#endif
