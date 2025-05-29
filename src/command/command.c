#include "command/command.h"
#include "command/builtins.h"
#include "core/process.h"
#include <utility.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int last_exit_code = 0;

int execute_command(Command *command) {
    char* input = strdup(command->command);
    char *token = strtok(input, " ");
    char *args[MAX_ARGUMENTS_SIZE];

    if (token == NULL) {
        // free dynamically allocated memory
        free(input);

        return 0;
    }

    args[0] = token;
    int i = 1;
    while ((token = strtok(NULL, " ")) != NULL && i < MAX_ARGUMENTS_SIZE - 1) {
        args[i++] = token;
    }
    args[i] = NULL;

    if (is_builtin_command(args[0])) {
        last_exit_code = run_builtin_command(args);
    } else {
        last_exit_code = run_child_process_piped(args, command);
    }

    free(input);
    return last_exit_code;
}
