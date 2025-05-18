#include <builtin.h>
#include <history.h>
#include <parser.h>
#include <process.h>
#include <utility.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int last_exit_code = 0;

int execute_command(char *input, unsigned int command_flags) {
    // TODO: valgrind throws if command is not found (e.g. "histoyr")
    char *original_input = strdup(input);
    input = convert_input(input);

    char *token = strtok(input, " ");
    char *args[MAX_ARGUMENTS_SIZE];
    bool is_history_command = *input == '!';

    if (token == NULL) {
        // free dynamically allocated memory
        free(original_input);
        free(input);

        return 0;
    }

    args[0] = token;
    int i = 1;
    while ((token = strtok(NULL, " ")) != NULL && i < MAX_ARGUMENTS_SIZE - 1) {
        args[i++] = token;
    }
    args[i] = NULL;

    if (is_history_command) {
        // convert the characters after the '!' to an unsigned long (by decimals), pass it to function
        last_exit_code = execute_command_from_history(strtoul(input + 1, NULL, 10));
    } else if (is_builtin_command(args[0])) {
        last_exit_code = run_builtin_command(args);
    } else {
        if (command_flags & CMD_FLAG_PIPE) {
            last_exit_code = run_child_process_piped(args);
        } else {
            last_exit_code = run_child_process_normal(args);
        }
    }

    if (!is_history_command) {
        append_to_history(original_input);
    }

    free(original_input);
    free(input);

    return last_exit_code;
}

