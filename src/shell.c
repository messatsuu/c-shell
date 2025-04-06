#define _POSIX_C_SOURCE 200809L  // Enables POSIX functions like strdup()

#include "../include/shell.h"
#include "../include/command.h"
#include "../include/input.h"
#include "unistd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#define INITIAL_BUFSIZE 20
#define BUF_EXPANSION_SIZE 100

extern int last_exit_code;

void set_env_field(char *special_field, size_t special_field_size, char *env_var) {
    char *env = getenv(env_var);
    if (env == NULL) {
        return;
    }

    strncpy(special_field, env, special_field_size);
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
    for (const char *pointer = ps1; *pointer != '\0'; pointer++) {
        // if the character isn't a '\', simply add it to ouput
        if (*pointer != '\\') {
            output[index++] = *pointer;
            continue;
        }

        // If the current character is '\', handle the char after it
        pointer++;

        // Skip bash escape sequences ('\]' & '\[') and newlines ('\n')
        if (*pointer == '[' || *pointer == ']' || *pointer == 'n') {
            continue;
        }

        if (*pointer == 'e' || strncmp(pointer, "033", 3) == 0) {
            output[index++] = '\033';
            if (*pointer == '0') {  // Skip "033"
                pointer += 2;
            }
            continue;
        }
        char special_field[256] = "";

        // Get the env variable and copy its contents to `special_field` (overrides existing contents with strncpy);
        switch (*pointer) {
            case 'u': {
                set_env_field(special_field, sizeof(special_field), "USER");
                break;
            }
            case 'w': {
                set_env_field(special_field, sizeof(special_field), "PWD");
                break;
            }
            case 'h': {
                gethostname(special_field, sizeof(special_field));
                break;
            }
            default:
                // Unknown sequence, keep it as is
                output[index++] = *pointer;
                continue;
        }

        // Concat the contents of `special_field` to output and move index forward
        strncat(output, special_field, strlen(special_field));
        index += strlen(special_field);
    }

    // null-terminate
    output[index++] = '\0';
    printf("%s", output);
}

void execute_input() {
    char *input = read_input();
    execute_command(input);
    free(input);
}

// flush terminal input and print PS1
void reset_shell() {
    tcflush(STDIN_FILENO, TCIFLUSH);

    printf("\n");
    create_ps1();
    fflush(stdout);
}

void create_prompt() {
    create_ps1();
    execute_input();
}
