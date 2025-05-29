#ifndef COMMAND_H
#define COMMAND_H

#include "parser/command_parser.h"
#include <unistd.h>

int execute_command(Command *command);

#endif
