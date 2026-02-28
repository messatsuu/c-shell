#include "core/init.h"
#include "core/shell.h"
#include "core/settings.h"
#include "parser/parser.h"
#include "utility.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

extern Settings *settings;

// On SIGINT, flush terminal input and print PS1
void handle_sigint(int signal) {
    reset_shell();
}

void setup_sigint_handler() {
    struct sigaction sig_action = {0};
    // Set the handler to be called on SIGINT
    sig_action.sa_handler = handle_sigint;
    // unset all flags on sigaction struct
    sigemptyset(&sig_action.sa_mask);

    // Bind the sigaction struct to the SIGINT signal
    if (sigaction(SIGINT, &sig_action, nullptr) == -1) {
        perror("sigaction error");
    }
}

void set_general_env_vars() {
    // set init-file path env-var
    const char *home = getenv("HOME");
    size_t len = strlen(home) + strlen("/.cshrc") + 1;
    char *init_file_path = allocate(len, true);

    snprintf(init_file_path, len, "%s/.cshrc", home);
    set_environment_var("CSHELL_INIT_PATH", init_file_path, true);
    free(init_file_path);

    // set the SHELL env-var to the path of the current executable
    char path[INITIAL_BUFSIZE_BIG];
    signed long length = readlink("/proc/self/exe", path, INITIAL_BUFSIZE_BIG);
    if (length == -1) {
        log_error_with_exit("Could not read shell executable name");
    }
    path[length] = '\0';
    set_environment_var("SHELL", path, true);

    // Set secondary prompt string (displayed on e.g. unclosed quotes)
    set_environment_var("PS2",  " > ", true);

    // Set the shell-level
    char *shell_level = getenv("SHLVL");
    char shell_level_str[INITIAL_BUFSIZE];
    long level = 1;

    if (shell_level) {
        // If env-var is found, add it to 1
        level += strtol(shell_level, nullptr, 10);
    }

    if (level > 1000) {
        log_error("warning: shell level (%d) too high, resetting to 1\n", level);
        level = 1;
    } else if (level < 0) {
        level = 0;
    }

    snprintf(shell_level_str, sizeof(shell_level_str), "%ld", level);
    set_environment_var("SHLVL", shell_level_str, true);
}

void parse_init_file() {
    char *init_file_path = getenv("CSHELL_INIT_PATH");
    if (!init_file_path) {
        return;
    }

    if (settings->debug_mode) {
        printf("Trying to source init file at %s\n", init_file_path);
    }

    int result = parse_interpretable_file(init_file_path);

    if (settings->debug_mode) {
        result == 0 ? printf("successfully parsed init-file.\n") : printf("Unable to parse init-file\n");
    }
}

// General function to do things before the shell is started
void initialize_shell() {
    init_settings();
    setup_sigint_handler();
    if (atexit(cleanup) != 0) {
        log_error_with_exit("Failed to set up exit-handler");
    }

    set_general_env_vars();
    parse_init_file();
}
