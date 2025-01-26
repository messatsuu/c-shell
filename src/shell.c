#include "../include/shell.h"
#include "../include/command.h"
#include "unistd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#define INITIAL_BUFSIZE 20

extern int last_exit_code;

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

void create_ps1() {
    char *ps1 = getenv("PS1");

    if (ps1 == NULL) {
        printf("$ ");
        return;
    }

    size_t index = 0;
    char output[100];

    // Example input: \n\[\033[1;32m\][nix-shell:\w]\$\[\033[0m\]
    // Example output: \033[1;32m[nix-shell:\w]$\033[0m
    for (const char *p = ps1; *p != '\0'; p++) {
        if (*p != '\\') {
            output[index++] = *p;
            continue;
        }

        // If the current character is '\', handle the next char
        p++;

        if (*p == '[' || *p == ']' || *p == 'n') {
            // Skip newlines and bash escape sequences (']' & '[')
            continue;
        } else if (*p == 'e') {
            output[index++] = '\033';
        } else {
            // TODO: fix expanding codes to env vars (e.g. u => $USER);
            // char *user = getenv("USER");
            // if (user == NULL) {
            //     continue;
            // }
            // int user_length = strlen(user);
            // strncat(output, user, user_length);
            // index += user_length;

            // Unknown, add it to output with the previous '\'
            output[index++] = '\\';
            output[index++] = *p;
        }
    }
    output[index++] = '\0';
    printf("%s", output);
}

void create_prompt() {
    while (1) {
        create_ps1();
        char *input = read_input();
        execute_command(input);
        free(input);
    }
}
