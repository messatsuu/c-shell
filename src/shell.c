#define _POSIX_C_SOURCE 200809L  // Enables POSIX functions like strdup()

#include "../include/shell.h"
#include "../include/command.h"
#include "../include/input.h"
#include "../include/parser.h"
#include "../include/prompt.h"
#include "unistd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

#define INITIAL_BUFSIZE 20
#define BUF_EXPANSION_SIZE 100

extern int last_exit_code;

void set_env_field(char *special_field, size_t special_field_size, char *env_var) {
    char *env = getenv(env_var);
    if (env == NULL) {
        return;
    }

    strncpy(special_field, env, special_field_size);
}


void execute_input() {
    int count = 0;
    // Commands gets dynamically allocated in `convert_input_to_commands()`
    Command *commands = NULL;
    bool should_run = true;

    char *input = read_input();
    convert_input_to_commands(input, &count, &commands);

    for (size_t i = 0; i < count; i++) {
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
            execute_command(command.command);
        }
        free(command.command);
    }

    free(commands);
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
