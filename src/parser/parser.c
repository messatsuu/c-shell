#include "parser/parser.h"
#include "cshread/history.h"
#include <utility.h>

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#define MAX_ENV_VAR_NAME_BUFSIZE 128

const char escapable_characters[] = {
    '\"',
    '\'',
    '(',
    '`' // TODO: this should invoke sub-context commands
};

const char quote_characters[] = {
    '\"',
    '\'',
};

// Convert an env variable from input to its value and concatenate into `buffer`.
// Return -1 on error, 0 on success
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

int convert_history_command(char **pointer, char **buffer, unsigned int *buffer_size, unsigned long *index) {
    // This also moves `pointer` to char* after the last number
    int history_index = strtoul(strchr(*pointer, '!') + 1, pointer, 0);
    char *command_from_history = cshr_history_get_entry_dup(history_index);
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

char get_escapable_character(char character) {
    for (unsigned int i = 0; i < (sizeof(escapable_characters) / sizeof(escapable_characters[0])); i++) {
        if (character == escapable_characters[i]) {
            return character;
        }
    }

    return -1;
}

char get_quote_character(char character) {
    for (unsigned int i = 0; i < (sizeof(quote_characters) / sizeof(quote_characters[0])); i++) {
        if (character == escapable_characters[i]) {
            return character;
        }
    }

    return -1;
}

void handle_quoted_string(char **pointer, char **buffer, unsigned int *buffer_size, unsigned long *index, bool remove_quotes, bool handle_vars) {
    // initial position is considered the quote-character
    char quote_character = **pointer;
    if (!remove_quotes) {
        (*buffer)[(*index)++] = **pointer;
    }

    // skip leading quote
    (*pointer)++;

    // Add characters to buffer until second quote is reached
    for (; **pointer != '\0';) {
        // Reallocate
        if (*index > *buffer_size - 1) {
            *buffer = reallocate_safe(*buffer, *buffer_size, *buffer_size + BUF_EXPANSION_SIZE, true);
            *buffer_size += BUF_EXPANSION_SIZE;
        }

        if (handle_vars && **pointer == '$') {
            convert_env_var(pointer, buffer, buffer_size, index);
            continue;
        }

        // Break on closing quote
        if (get_escapable_character(**pointer) == quote_character) {
            if (!remove_quotes) {
                (*buffer)[(*index)++] = **pointer;
                (*pointer)++;
            }
            break;
        }

        (*buffer)[(*index)++] = **pointer;
        (*pointer)++;
    }
}

void mutate_original_input(char **input) {
    unsigned int buffer_size = INITIAL_BUFSIZE;
    char *buffer = callocate(buffer_size, 1, true);
    unsigned long index = 0;

    for (char *pointer = *input; *pointer != '\0';) {
        // If the buffer is full, reallocate memory
        if (index >= buffer_size - 1) {
            char *temp = reallocate_safe(buffer, buffer_size, buffer_size + BUF_EXPANSION_SIZE, false);
            buffer_size += BUF_EXPANSION_SIZE;

            if (!temp) {
                log_error("Input buffer Reallocation Error");
                input = nullptr;
                return;
            }

            buffer = temp;
        }

        if (*pointer == '\\' && get_escapable_character(*(pointer + 1)) != 0) {
            buffer[index++] = *pointer;
            pointer++;
            buffer[index++] = *pointer;
            pointer++;

            continue;
        }

        char quote_character = get_escapable_character(*pointer);
        if (quote_character) {
            handle_quoted_string(&pointer, &buffer, &buffer_size, &index, false, false);
            continue;
        }

        if (*pointer == '!') {
            if (convert_history_command(&pointer, &buffer, &buffer_size, &index) == -1) {
                free(buffer);
                free(*input);
                *input = nullptr;
                return;
            }
            continue;
        }

        // If the input is not a "special character", just add it to the output
        buffer[index++] = *pointer;
        pointer++;
    }

    if (index > buffer_size) {
        log_error("Buffer overflow detected when terminating input string.");
        free(buffer);
        *input = nullptr;
        return;
    }

    free(*input);
    *input = buffer;
}

void convert_argv(char **argv) {
    unsigned int i = 0;

    while (argv[i] != nullptr) {
        char *argument = argv[i];

        unsigned int buffer_size = INITIAL_BUFSIZE;
        char *buffer = callocate(buffer_size, 1, true);
        unsigned long index = 0;
        for (char *pointer = argument; *pointer != '\0';) {
            // If the buffer is full, reallocate memory
            if (index >= buffer_size - 1) {
                char *temp = reallocate_safe(buffer, buffer_size, buffer_size + BUF_EXPANSION_SIZE, false);
                buffer_size += BUF_EXPANSION_SIZE;

                if (!temp) {
                    log_error("Input buffer Reallocation Error");
                    // return nullptr;
                }

                buffer = temp;
            }

            if (*pointer == '\\' && get_escapable_character(*(pointer + 1)) != -1) {
                buffer[index++] = *(pointer + 1);
                pointer += 2;
                continue;
            }

            char quote_character = get_quote_character(*pointer);
            if (quote_character != -1) {
                bool handle_vars = true;
                if (quote_character == '\'') {
                    handle_vars = false;
                }
                handle_quoted_string(&pointer, &buffer, &buffer_size, &index, true, handle_vars);
                continue;
            }


            if (*pointer == '~') {
                char *home_env_var = getenv("HOME");
                if (home_env_var != NULL) {
                    strcat(buffer, home_env_var);
                    pointer++;
                    index += strlen(home_env_var);
                    continue;
                }
            }

            if (*pointer == '$') {
                if (convert_env_var(&pointer, &buffer, &buffer_size, &index) == -1) {
                    // TODO: handle this correctly
                    // free(buffer);
                    // return nullptr;
                }
                continue;
            }

            // If the input is not a "special character", just add it to the output
            buffer[index++] = *pointer;
            pointer++;
        }

        free(argv[i]);
        buffer[index] = '\0';
        argv[i++] = buffer;
    }
}
