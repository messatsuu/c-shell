#include "command/builtins.h"
#include "command/alias.h"
#include "cshread/history.h"
#include <utility.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define MAX_ENV_VAR_NAME 255
#define MAX_ENV_VAR_VALUE 65536 // 64 KB

extern int last_exit_code;
bool continue_execution = true;

int run_builtin_cd(char **argv) {
    char *path = argv[1];
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

    return 0;
}

int run_builtin_history(char **argv) {
    return cshr_print_history();
}

int run_builtin_exit(char **argv) {
    continue_execution = false;
    return 0;
}

int run_builtin_export(char **argv) {
    char *equal_sign = strchr(argv[1], '=');
    if (equal_sign == NULL) {
        return 0;
    }

    unsigned int name_len = equal_sign - argv[1];
    if (name_len >= MAX_ENV_VAR_NAME) {
        log_error("export: environment variable name too long");
        return 1;
    }

    char env_var_name[MAX_ENV_VAR_NAME] = {0};
    char env_var_value[MAX_ENV_VAR_VALUE] = {0};

    strncpy(env_var_name, argv[1], name_len);
    env_var_name[name_len] = '\0';
    strncpy(env_var_value, equal_sign + 1, MAX_ENV_VAR_VALUE - 1);
    env_var_value[MAX_ENV_VAR_VALUE - 1] = '\0';

    setenv(env_var_name, env_var_value, 1);

    return 0;
}

int run_builtin_alias(char **argv) {
    if (argv[1] == nullptr) {
        return print_aliases();
    }

    unsigned int argc = 1;
    while (argv[argc] != nullptr) {

        char *equal_sign = strchr(argv[argc], '=');
        if (equal_sign == nullptr) {
            if (print_alias(argv[argc++]) != 0) {
                return 1;
            }
            continue;
        }

        // equal-sign found in argv, add alias
        char alias_name[MAX_ALIAS_NAME] = {0};
        char command[MAX_ALIAS_COMMAND] = {0};

        strncpy(alias_name, argv[argc], equal_sign - argv[argc]);
        strncpy(command, equal_sign + 1, MAX_ALIAS_COMMAND - 1);

        if (add_alias(alias_name, command) != 0) {
            return 1;
        }
        argc++;
    }

    return 0;
}

int run_builtin_unalias(char **argv) {
    if (argv[1] == nullptr) {
        printf("unalias: usage: unalias [-a] name [name ...]\n");
        return 1;
    }

    if (strcmp(argv[1], "-a") == 0) {
        return clear_aliases();
    }

    unsigned int argc = 1;
    while (argv[argc] != nullptr) {
        if (remove_alias(argv[argc++]) != 0) {
            return 1;
        }
    }

    return 0;
}

const BuiltinCommand builtin_commands[] = {
    {"cd", run_builtin_cd},
    {"history", run_builtin_history },
    {"exit", run_builtin_exit },
    {"export", run_builtin_export },
    {"alias", run_builtin_alias},
    {"unalias", run_builtin_unalias},
};

// bytes in struct-array divided by number of bytes of a single struct is the number of pointers
const unsigned int number_of_builtin_commands = sizeof(builtin_commands) / sizeof(builtin_commands[0]);

const BuiltinCommand *get_builtin_command(const char *command) {
    for (size_t i = 0; i < number_of_builtin_commands; i++) {
        if (strcmp(command, builtin_commands[i].name) == 0) {
            return &builtin_commands[i];
        }
    }

    return nullptr;
}

bool is_builtin_command(const char *command) {
    return get_builtin_command(command) != nullptr;
}

int run_builtin_command(char **argv) {
    const BuiltinCommand *builtinCommand = get_builtin_command(argv[0]);
    if (builtinCommand == nullptr) {
        log_error("Reported command %s is not a builtin", argv[0]);
        return 0;
    }

    return builtinCommand->handler(argv);
}
