#include "command/command.h"
#include "input/history.h"
#include "command/builtins.h"
#include "core/process.h"
#include "parser/command_parser.h"
#include <utility.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int last_exit_code = 0;

int execute_command(Command *command) {
    char* input = strdup(command->command);
    if (command->flags & CMD_FLAG_HISTORY) {
        // convert the characters after the '!' to an unsigned long (by decimals), pass it to function
        input = get_command_from_history(strtoul(strchr(input, '!') + 1, NULL, 10));
    }

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
        last_exit_code = run_child_process_piped(args, command->flags);
    }

    free(input);
    return last_exit_code;
}
