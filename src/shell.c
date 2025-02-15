#include "../include/shell.h"
#include "../include/command.h"
#include "unistd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#define INITIAL_BUFSIZE 20

extern int last_exit_code;

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
            // newline determines end of input, return
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

    // If PS1 is not set, we use a default prompt
    if (ps1 == NULL) {
        ps1 = "\e[0;34mnic-shell \\u@\\h> \e[m";
    }

    size_t index = 0;
    // Make sure that `output` is initialized as "", since the function is called multiple times (may lead to undefined behavior)
    char output[100] = "";

    // Example input: \n\[\033[1;32m\][nix-shell:\w]\$\[\033[0m\]
    // Example output: \033[1;32m[nix-shell:\w]$\033[0m
    for (const char *p = ps1; *p != '\0'; p++) {
        // if the character isn't a '\', simply add it to ouput
        if (*p != '\\') {
            output[index++] = *p;
            continue;
        }

        // If the current character is '\', handle the next char
        p++;

        // Skip bash escape sequences ('\]' & '\[') and newlines ('\n')
        if (*p == '[' || *p == ']' || *p == 'n') {
            continue;
        } else if (*p == 'e' || strncmp(p, "033", 3) == 0) {
            output[index++] = '\033';
            if (*p == '0') {  // Skip "033"
                p += 2;
            }
            continue;
        }
        char special_field[256] = "";

        // Get the env variable and copy its contents to `special_field` (overrides existing contents with strncpy);
        switch (*p) {
            case 'u': {
                char *user = getenv("USER");
                strncpy(special_field, user, sizeof(special_field));
                break;
            }
            case 'w': {
                char *pwd = getenv("PWD");
                strncpy(special_field, pwd, sizeof(special_field));
                break;
            }
            case 'h': {
                gethostname(special_field, sizeof(special_field));
                break;
            }
            default:
                // Unknown sequence, keep it as is
                output[index++] = *p;
                continue;
        }

        // Concat the contents of `special_field` to output and move index forward
        strncat(output, special_field, strlen(special_field));
        index += strlen(special_field);
    }
    output[index++] = '\0';
    printf("%s", output);
}

// flush terminal input and print PS1
void reset_shell() {
    tcflush(STDIN_FILENO, TCIFLUSH);

    printf("\n");
    create_ps1();
    fflush(stdout);
}


void create_prompt() {
    while (1) {
        create_ps1();
        char *input = read_input();
        execute_command(input);
        free(input);
    }
}
