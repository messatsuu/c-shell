#include "parser/parser.h"
#include "input/history.h"
#include <utility.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#define MAX_ENV_VAR_NAME_BUFSIZE 128

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

int convert_history_command(char **pointer, char *input, char **buffer, unsigned int *buffer_size, unsigned long *index) {
    // This also moves `pointer` to char* after the last number
    int history_index = strtoul(strchr(*pointer, '!') + 1, pointer, 0);
    char *command_from_history = get_command_from_history(history_index);
    if (command_from_history == NULL) {
        return -1;
    }

    int command_length = strlen(command_from_history);

    if (*index + command_length >= *buffer_size - 1) {
        // TODO: handle realloc error gracefully
        while ((*buffer_size - 1) < *index + command_length) {
            *buffer_size += BUF_EXPANSION_SIZE;
        }
        *buffer = reallocate(*buffer, *buffer_size, true);
    }

    strcat(*buffer, command_from_history);
    free(command_from_history);
    *index += command_length;

    return 0;
}

char *convert_input(char *input) {
    unsigned int buffer_size = INITIAL_BUFSIZE;
    char *buffer = callocate(buffer_size, 1, true);
    unsigned long index = 0;

    for (char *pointer = input; *pointer != '\0';) {
        // If the buffer is full, reallocate memory
        if (index >= buffer_size - 1) {
            buffer_size += BUF_EXPANSION_SIZE;
            char *temp = reallocate(buffer, buffer_size, false);

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

        if (*pointer == '!') {
            if (convert_history_command(&pointer, input, &buffer, &buffer_size, &index) == -1) {
                free(buffer);
                return NULL;
            }
            continue;
        }

        // If the input is not a "special character", just add it to the output
        buffer[index++] = *pointer;
        pointer++;
    }

    if (index >= buffer_size) {
        log_error("Buffer overflow detected when terminating input string.");
        free(buffer);
        return NULL;
    }

    buffer[index] = '\0';
    return buffer;
}

char ***create_piped_command_array(char *args[]) {
    char*** commands = allocate(sizeof(char**) * MAX_COMMANDS, true);
    int command_index = 0;
    int argument_index = 0;

    commands[command_index] = allocate(sizeof(char*) * MAX_ARGS_PER_COMMAND, true);

    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            // Null terminate end of command
            commands[command_index][argument_index] = NULL;
            // Start a new command
            command_index++;
            argument_index = 0;
            commands[command_index] = allocate(sizeof(char*) * MAX_ARGS_PER_COMMAND, true);
            continue;
        }

        commands[command_index][argument_index++] = args[i];
    }

    // NULL-terminate last command
    commands[command_index][argument_index] = NULL;

    // NULL-terminate the command array
    commands[command_index + 1] = NULL;

    return commands;
}
