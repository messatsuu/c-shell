#define _POSIX_C_SOURCE 200809L  // Enables POSIX functions like strdup()

#include "../include/history.h"
#include "../include/process.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_ARGUMENTS_SIZE 100

const char *builtin_commands[] = {"cd", "history", "exit", "last_exit_code"};
// bytes in pointer array divided by nr of bites of a single pointer is the number of pointers
const int number_of_builtin_commands = sizeof(builtin_commands) / sizeof(builtin_commands[0]);
int last_exit_code = 0;

bool is_builtin_command(const char *command) {
    #pragma unroll 4
    for (size_t i = 0; i < number_of_builtin_commands; i++) {
        if (strcmp(command, builtin_commands[i]) == 0) {
            return true;
        }
    }

    return false;
}

int run_builtin_command(char *command[]) {
    if (strcmp("cd", command[0]) == 0) {
        char *path = command[1];
        if (path == NULL) {
            path = getenv("HOME");

            if (path == NULL) {
                if (fprintf(stderr, "cd: HOME environment variable is not set\n") < 0) {
                    perror("Error writing to stderr.");
                }

                return 1;
            }
        }

        if (chdir(path) != 0) {
            perror("Error changing directory");
            return 1;
        }
    } else if (strcmp("exit", command[0]) == 0) {
        exit(0);
    } else if (strcmp("history", command[0]) == 0) {
        print_history();
    } else if (strcmp("last_exit_code", command[0]) == 0) {
        printf("Last exit code: %d\n", last_exit_code);
    }

    // TODO: implement `export`
    return 0;
}

int execute_command(char *input) {
    char *original_input = strdup(input);
    char *token = strtok(input, " ");
    char *args[MAX_ARGUMENTS_SIZE];
    // If the first character of the input is a '!' we know that it is trying to call a command from the history
    bool is_history_command = *input == '!';

    if (token == NULL) {
        return 0;
    }

    args[0] = strdup(token);

    int i = 1;
    while ((token = strtok(NULL, " ")) != NULL && i < MAX_ARGUMENTS_SIZE - 1) {
        args[i++] = token;
    }
    args[i] = NULL;

    if (is_history_command) {
        // convert the characters after the '!' to an unsigned long (by decimals), pass it to function
        execute_command_from_history(strtoul(input + 1, NULL, 10));
    } else if (is_builtin_command(args[0])) {
        last_exit_code = run_builtin_command(args);
    } else {
        last_exit_code = run_child_process(args);
    }

    if (last_exit_code == 0 && !is_history_command) {
        append_to_history(original_input);
    }

    // strdup() allocates memory that needs to be freed
    free(args[0]);
    free(original_input);
    return last_exit_code;
}

