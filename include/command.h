#ifndef COMMAND_H
#define COMMAND_H

#include <stdbool.h>
#include <unistd.h>

bool is_builtin_command(const char *command);
int run_builtin_command(char *command[]);
int execute_command(char *input);

#endif
