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
    unsigned int historyIndex;
    unsigned int buffer_size;
} InputBuffer;

#endif
