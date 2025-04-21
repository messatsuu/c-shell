#define _POSIX_C_SOURCE 200809L  // Enables POSIX functions like strdup()

#include "../include/utility.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#define INITIAL_COMMAND_CAPACITY 1
#define INITIAL_BUFSIZE 20
#define BUF_EXPANSION_SIZE 100
#define MAX_ENV_VAR_NAME_BUFSIZE 128

// Separators
#define SEQUENTIAL_SEPARATOR = 0
#define AND_SEPARATOR = 1
#define OR_SEPARATOR = 2

char *read_input() {
    unsigned int buffer_size = INITIAL_BUFSIZE;
    unsigned int length = 0;

    char *buffer = malloc(buffer_size);
    if (!buffer) {
        log_error_with_exit("Buffer Allocation Error");
    }

    char *buffer_pointer = buffer;

    // read (current size of buffer - current length of input) into buffer
    while (fgets(buffer_pointer, buffer_size - length, stdin)) {
        length += strlen(buffer_pointer);

        if (buffer[length - 1] == '\n') {
            buffer[length - 1] = '\0';
            // newline determines end of input, return
            return buffer;
        }

        // If no newline was found at the end of the string means that the string
        // hasn't ended yet and buffer is full, expand buffer by `BUF_EXPANSION_SIZE` bytes
        buffer_size += BUF_EXPANSION_SIZE;
        buffer = reallocate(buffer, buffer_size, false);
        if (!buffer) {
            return NULL;
        }

        // set pointer to end of buffer, so next chunk read appends to end
        buffer_pointer = buffer + length;
    }

    if (feof(stdin)) {
        free(buffer);
        exit(EXIT_FAILURE);
    }

    if (ferror(stdin)) {
        log_error("Error reading input");
        free(buffer);
        return NULL;
    }

    return buffer;
}
