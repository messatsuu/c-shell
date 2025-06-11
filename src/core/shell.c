#include "command/command.h"
#include "core/shell.h"
#include "parser/command_parser.h"
#include "parser/parser.h"
#include <cshread/cshread.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

extern int last_exit_code;

void execute_input() {
    int command_count = 0;
    // Commands gets dynamically allocated in `convert_input_to_commands()`
    Command *commands = nullptr;
    bool should_run = true;
    char *original_input = cshr_read_input();

    if (original_input == NULL) {
        free(commands);
        exit(0);
    }

    mutate_original_input(&original_input);
    char *input = convert_input(original_input);

    if (input == NULL) {
        return;
    }

    convert_input_to_commands(input, &command_count, &commands);
    // Command duplicates the needed parts of input, so we free early

    for (size_t i = 0; i < command_count; i++) {
        Command command = commands[i];

        if (i != 0) {
            char *previous_separator = commands[i - 1].separator;

            if (strcmp(previous_separator, "&&") == 0) {
                should_run = (last_exit_code == 0);
            } else if (strcmp(previous_separator, "||") == 0) {
                should_run = (last_exit_code != 0);
            } else if (strcmp(previous_separator, ";") == 0) {
                should_run = true;
            }
        }

        if (should_run) {
            execute_command(&command);
        }
        free(command.command);
    }

    // TODO: history command (!10) should store evaluated command, not literal input
    chsr_history_append(original_input);
    free(original_input);
    free(commands);
    free(input);
}

// flush terminal input and print PS1
void reset_shell() {
    printf("\n");
    fflush(stdout);
    // sigint_received = 1;
}

void create_prompt() {
    execute_input();
}
