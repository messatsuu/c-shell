#define _POSIX_C_SOURCE 200809L  // Enables POSIX functions like strdup()

#include "../include/shell.h"
#include "../include/command.h"
#include "../include/utility.h"
#include "unistd.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#define INITIAL_BUFSIZE 20
#define BUF_EXPANSION_SIZE 100
#define MAX_CONVERTED_BUFSIZE 10024
#define MAX_ENV_VAR_NAME_BUFSIZE 128

extern int last_exit_code;

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
        buffer = realloc(buffer, buffer_size);

        if (!buffer) {
            log_error("Input Buffer Allocation Error");
            free(buffer);
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

char *convert_input(const char *input) {
    unsigned int buffer_size = INITIAL_BUFSIZE;
    char *buffer = malloc(buffer_size);
    // Fill buffer with 0s to avoid garbage values in output
    memset(buffer, 0, buffer_size);

    size_t index = 0;

    for (const char *pointer = input; *pointer != '\0';) {
        // If the buffer is full, reallocate memory
        if (strlen(buffer) > buffer_size) {
            buffer_size += BUF_EXPANSION_SIZE;
            char *temp = realloc(buffer, buffer_size);

            if (!buffer) {
                log_error("Input buffer Reallocation Error");
                return NULL;
            }

            buffer = temp;
        }

        // If the input is not a '$', just add it to the output
        if (*pointer != '$') {
            buffer[index++] = *pointer;
            pointer++;
            continue;
        }

        // If the character starts with a '$', we expect an env var
        pointer++;

        // Read the name of the env-var into a new buffer
        char env_var_name[MAX_ENV_VAR_NAME_BUFSIZE] = "";
        size_t env_var_index = 0;
        while (isalnum(*pointer) || *pointer == '_') {
            env_var_name[env_var_index] = *pointer;

            env_var_index++;
            pointer++;
        }

        // Get the value of the env-var
        char *env_var_value = getenv(env_var_name);
        if (env_var_value == NULL) {
            continue;
        }
        unsigned long env_var_length = strlen(env_var_value);

        // If the current value + the value we want to put into the buffer is bigger than its size, reallocate memory for it
        if (strlen(buffer) + env_var_length + 1 > buffer_size) {
            buffer_size += env_var_length + 1;
            buffer = realloc(buffer, buffer_size);

            if (!buffer) {
                log_error("Input buffer Allocation Error");
                return NULL;
            }
        }

        // Add the value of the env-var to the buffer
        strncat(buffer, env_var_value, env_var_length + 1);
        index += strlen(env_var_value);
    }

    buffer[index] = '\0';
    return buffer;
}

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
    input = convert_input(input);

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
