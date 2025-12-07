#ifndef BUILTIN_H
#define BUILTIN_H

#include <stdbool.h>
#include <unistd.h>

bool is_builtin_command(const char *command);
int run_builtin_command(char **argv);

#endif
