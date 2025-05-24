#include <command.h>
#include <input.h>
#include <parser.h>
#include <prompt.h>
#include <shell.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <stdbool.h>

extern int last_exit_code;

void set_env_field(char *special_field, size_t special_field_size, char *env_var) {
    char *env = getenv(env_var);
    if (env == NULL) {
        return;
    }

    strncpy(special_field, env, special_field_size);
}

void execute_input() {
    int command_count = 0;
    // Commands gets dynamically allocated in `convert_input_to_commands()`
    Command *commands = NULL;
    bool should_run = true;
    char *input = read_input_prompt();

    if (input == NULL) {
        free(commands);
        exit(0);
    }

    convert_input_to_commands(input, &command_count, &commands);

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
            execute_command(command.command, command.flags);
        }
        free(command.command);
    }

    free(commands);
    free(input);
}

// flush terminal input and print PS1
void reset_shell() {
    printf("\n");
    fflush(stdout);
    sigint_received = 1;
}

void create_prompt() {
    create_ps1();
    printf("%s", prompt);
    execute_input();
}
