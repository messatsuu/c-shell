#ifndef PROCESS_H
#define PROCESS_H

#include "command/command.h"

int run_child_process_normal(char *args[]);

int run_child_process_piped(char *args[], Command *command);

#endif
