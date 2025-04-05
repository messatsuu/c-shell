#ifndef HISTORY_H
#define HISTORY_H

void init_history(int initial_capacity);

void append_to_history(const char *command);
void print_history();
int execute_command_from_history(unsigned long index);

#endif
