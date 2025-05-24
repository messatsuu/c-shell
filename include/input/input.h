#ifndef INPUT_H
#define INPUT_H

#include <signal.h>

extern volatile sig_atomic_t sigint_received;

// Reads input from the user
char *read_input_prompt();

typedef struct {
    char *buffer;
    char *buffer_backup;
    unsigned int length;
    unsigned int cursor_position;
    unsigned int history_index;
    unsigned int buffer_size;
} InputBuffer;

#endif

void insert_into_buffer_at_cursor_position(InputBuffer *inputBuffer, char *string, unsigned int string_length);
