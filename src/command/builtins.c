#include "command/command.h"
#include "input/history.h"
#include <utility.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define MAX_ENV_VAR_NAME 255
#define MAX_ENV_VAR_VALUE 65536 // 64 KB

extern int last_exit_code;

const char *builtin_commands[] = {
    "cd",
    "history",
    "exit",
    "last_exit_code",
    "export"
};
// bytes in pointer array divided by nr of bites of a single pointer is the number of pointers
const int number_of_builtin_commands = sizeof(builtin_commands) / sizeof(builtin_commands[0]);

bool is_builtin_command(const char *command) {
    #pragma unroll 5
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

        // Update $PWD to the current directory
        char cwd[1000];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            perror("getcwd");
            return 1;
        }

        setenv("PWD", cwd, 1);
    } else if (strcmp("exit", command[0]) == 0) {
        // TODO: when exiting here, memory from execute_command() does not get freed
        exit(0);
    } else if (strcmp("history", command[0]) == 0) {
        print_history();
    } else if (strcmp("last_exit_code", command[0]) == 0) {
        printf("Last exit code: %d\n", last_exit_code);
    } else if (strcmp("export", command[0]) == 0) {
        // TODO: Change parser.c to work with "". Currently `export foo=";bar"` breaks
        char *equal_sign = strchr(command[1], '=');
        if (equal_sign == NULL) {
            return 0;
        }

        size_t name_len = equal_sign - command[1];
        if (name_len >= MAX_ENV_VAR_NAME) {
            log_error("export: environment variable name too long");
            return 1;
        }

        char env_var_name[MAX_ENV_VAR_NAME] = {0};
        char env_var_value[MAX_ENV_VAR_VALUE] = {0};

        strncpy(env_var_name, command[1], name_len);
        env_var_name[name_len] = '\0';
        strncpy(env_var_value, equal_sign + 1, MAX_ENV_VAR_VALUE - 1);
        env_var_value[MAX_ENV_VAR_VALUE - 1] = '\0';

        setenv(env_var_name, env_var_value, 1);
    }

    return 0;
}

