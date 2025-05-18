#ifndef COMMAND_H
#define COMMAND_H

#include <parser.h>

#include <unistd.h>

int execute_command(char *input, unsigned int command_flags);

#endif
