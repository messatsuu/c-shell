#ifndef HISTORY_H
#define HISTORY_H

typedef struct {
    char **entries; // Array of strings (dynamically allocated)
    int count;      // Number of current entries
    int capacity;   // Number of entries until reallocation is needed
} History;

void init_history(int initial_capacity);
void cleanup_history();

void append_to_history(const char *command);
void print_history();
int execute_command_from_history(unsigned long index);

#endif
