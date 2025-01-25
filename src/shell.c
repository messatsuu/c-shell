#include "../include/shell.h"
#include "../include/command.h"
#include "unistd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#define INITIAL_BUFSIZE 20

// flush terminal input and print PS1
void reset_shell() {
    tcflush(STDIN_FILENO, TCIFLUSH);

    printf("\n$ ");
    fflush(stdout);
}

char *read_input() {
    size_t buffer_size = INITIAL_BUFSIZE;
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }

    char *buffer_pointer = buffer;
    size_t length = 0;

    while (fgets(buffer_pointer, buffer_size - length, stdin)) {
        length += strlen(buffer_pointer);
        // remove newline
        if (buffer[length - 1] == '\n') {
            buffer[length - 1] = '\0';
            return buffer;
        }

        // Buffer full, add 100 bytes
        buffer_size += 100;
        buffer = realloc(buffer, buffer_size);
        if (!buffer) {
            fprintf(stderr, "Allocation error\n");
            exit(EXIT_FAILURE);
        }
        buffer_pointer = buffer + length;
    }

    if (feof(stdin)) {
        free(buffer);
        exit(0);
    }

    if (ferror(stdin)) {
        fprintf(stderr, "Error reading input\n");
        free(buffer);
        return NULL;
    }

    return buffer;
}

void create_prompt() {
    while (1) {
        printf("$ ");
        char *input = read_input();
        execute_command(input);
        free(input);
    }
}
