#define _POSIX_C_SOURCE 200809L  // Enables POSIX functions like strdup()

#include "../include/utility.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#define INITIAL_COMMAND_CAPACITY 1
#define MAX_ENV_VAR_NAME_BUFSIZE 128

typedef struct {
    char *command;
    char separator[3];
} Command;

int convert_env_var(char **pointer, char **buffer, unsigned int *buffer_size, unsigned long *index) {
    // Skip '$' character
    (*pointer)++;

    // Read the name of the env-var into a new buffer
    char env_var_name[MAX_ENV_VAR_NAME_BUFSIZE] = "";
    size_t env_var_index = 0;
    while (isalnum(**pointer) || **pointer == '_') {
        env_var_name[env_var_index] = **pointer;

        env_var_index++;
        (*pointer)++;
    }

    // Get the value of the env-var
    char *env_var_value = getenv(env_var_name);
    if (env_var_value == NULL) {
        return 0;
    }
    unsigned long env_var_length = strlen(env_var_value);

    // If the current value + the value we want to put into the buffer is bigger than its size, reallocate memory for it
    if (strlen(*buffer) + env_var_length + 1 > *buffer_size) {
        *buffer_size += env_var_length + 1;
        *buffer = reallocate(*buffer, *buffer_size, false);

        if (!*buffer) {
            return -1;
        }
    }

    // Add the value of the env-var to the buffer
    strncat(*buffer, env_var_value, env_var_length + 1);
    *index += env_var_length;

    return 0;
}

void convert_input_to_commands(char *input, int *count, Command **commands) {
    int command_capacity = INITIAL_COMMAND_CAPACITY;

    if (*commands == NULL) {
        *commands = malloc(command_capacity * sizeof(Command));
    }

    for (const char *pointer = input; *pointer != '\0';) {
        if (*count >= command_capacity) {
            command_capacity += INITIAL_COMMAND_CAPACITY;
            *commands = reallocate(*commands, command_capacity * sizeof(Command), true);
        }

        const char *sep = strstr(pointer, "&&");
        const char *sep2 = strstr(pointer, "||");
        const char *sep3 = strchr(pointer, ';');

        // Find the nearest separator
        const char *nearest = NULL;
        const char *selected = NULL;
        const char *separators[] = {sep, sep2, sep3};
        const char *names[] = {"&&", "||", ";"};

        for (int j = 0; j < 3; j++) {
            if (separators[j] && (!nearest || separators[j] < nearest)) {
                nearest = separators[j];
                selected = names[j];
            }
        }

        // If there's a starting whitespace, remove it
        if (*pointer == ' ') {
            pointer++;
        }

        if (!nearest) {
            // No more separators, grab the rest
            (*commands)[*count].command = strdup(pointer);
            (*commands)[*count].separator[0] = '\0';  // No separator after the last command
            *count = *count + 1;
            break;
        }

        // TODO: Do both arguments need to be dynamically allocated?
        (*commands)[*count].command = strndup(pointer, nearest - pointer);
        strncpy((*commands)[*count].separator, selected, sizeof((*commands)[*count].separator));
        *count = *count + 1;

        pointer = nearest + strlen(selected);
    }
}

char *convert_input(char *input) {
    unsigned int buffer_size = INITIAL_BUFSIZE;
    char *buffer = callocate(buffer_size, 1, true);
    // Fill buffer with 0s to avoid garbage values in output
    memset(buffer, 0, buffer_size);

    size_t index = 0;

    for (char *pointer = input; *pointer != '\0';) {
        // If the buffer is full, reallocate memory
        if (index >= buffer_size -1) {
            buffer_size += BUF_EXPANSION_SIZE;
            char *temp = realloc(buffer, buffer_size);

            if (!temp) {
                log_error("Input buffer Reallocation Error");
                return NULL;
            }

            buffer = temp;
        }

        if (*pointer == '$') {
            if (convert_env_var(&pointer, &buffer, &buffer_size, &index) == -1) {
                free(buffer);
                return NULL;
            }
            continue;
        }

        // If the input is not a "special character", just add it to the output
        buffer[index++] = *pointer;
        pointer++;
    }

    if (index < buffer_size) {
        buffer[index] = '\0';
    } else {
        log_error("Buffer overflow detected when terminating input string.");
        free(buffer);
        return NULL;
    }

    return buffer;
}
