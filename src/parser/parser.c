#include "parser/parser.h"
#include <utility.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#define MAX_ENV_VAR_NAME_BUFSIZE 128

void set_command_flags(Command *command) {
    if (strchr(command->command, '|')) {
        command->flags |= CMD_FLAG_PIPE;
    }
    if (strchr(command->command, '>')) {
        command->flags |= CMD_FLAG_REDIRECT;
    }
    // TODO: implement CMD_FLAG_BACKGROUND
}

void initialize_command(Command *command) {
    command->flags = 0;
    command->separator[0] = '\0';
}

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
    int command_capacity = INITIAL_BUFSIZE;

    if (*commands == NULL) {
        *commands = callocate(command_capacity, sizeof(Command), true);
    }

    for (const char *pointer = input; *pointer != '\0';) {
        if (*count >= command_capacity) {
            command_capacity += BUF_EXPANSION_SIZE;
            *commands = reallocate(*commands, command_capacity * sizeof(Command), true);
        }

        // TODO: make it so that `echo "this ; is ; a string"` is a single command
        // TODO: Implement escpaing space separation for arguments (e.g. xdg-open My\ File.zip)
        const char *and_separator = strstr(pointer, "&&");
        const char *or_separator = strstr(pointer, "||");
        const char *semi_separator = strchr(pointer, ';');

        const char *next_separator = NULL;
        const char *selected = NULL;
        const char *separators[] = {and_separator, or_separator, semi_separator};
        const int number_of_separators = sizeof(separators) / sizeof(separators[0]);
        const char *names[] = {"&&", "||", ";"};

        Command *command = &(*commands)[*count];

        // Find the nearest separator
        for (int i = 0; i < number_of_separators; i++) {
            if (separators[i] != NULL && (next_separator == NULL || separators[i] < next_separator)) {
                next_separator = separators[i];
                selected = names[i];
            }
        }

        // If there's a starting whitespace, remove it
        if (*pointer == ' ') {
            pointer++;
        }

        *count = *count + 1;
        initialize_command(command);

        if (!next_separator) {
            // No more separators, grab the rest (until end of string) and finish
            command->command = strdup(pointer);
            set_command_flags(command);
            break;
        }

        command->command = strndup(pointer, next_separator - pointer);
        strncpy(command->separator, selected, sizeof(command->separator));
        set_command_flags(command);

        // Continue after the separator
        pointer = next_separator + strlen(selected);
    }
}

char *convert_input(char *input) {
    unsigned int buffer_size = INITIAL_BUFSIZE;
    char *buffer = callocate(buffer_size, 1, true);
    size_t index = 0;

    for (char *pointer = input; *pointer != '\0';) {
        // If the buffer is full, reallocate memory
        if (index >= buffer_size -1) {
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
