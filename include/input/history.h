#ifndef HISTORY_H
#define HISTORY_H

typedef struct {
    char **entries; // Array of strings (dynamically allocated)
    unsigned int count;      // Number of current entries
    unsigned int capacity;   // Number of entries until reallocation is needed
} History;

extern int history_index;

void init_history(int initial_capacity);

void cleanup_history();

void append_to_history(const char *command);

char* get_command_from_history(unsigned long index);

char *get_entry_from_history(const unsigned int index);

void print_history();

#endif
