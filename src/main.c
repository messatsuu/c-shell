#include "core/shell.h"
#include "core/prompt.h"

#include <cshread/cshread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utility.h>
#include <wait.h>

extern bool continue_execution;
extern int last_exit_code;

// On SIGINT, flush terminal input and print PS1
void handle_sigint(int signal) {
    reset_shell();
}

void setup_signal_handlers() {
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

// General function to do things before the shell is started
void initialize_shell() {
    // set the SHELL env-var to the path of the current executable
    char path[INITIAL_BUFSIZE_BIG];
    signed long length = readlink("/proc/self/exe", path, INITIAL_BUFSIZE_BIG);
    if (length == -1) {
        log_error_with_exit("Could not read shell executable name");
    }

    path[length] = '\0';
    setenv("SHELL", path, true);

    // Set secondary prompt string (displayed on e.g. unclosed quotes)
    setenv("PS2",  " > ", true);

    // Set the shell-level correctly
    char *shell_level = getenv("SHLVL");
    char shell_level_str[INITIAL_BUFSIZE];
    long level = 1;

    if (shell_level) {
        // If env-var is found, add it to 1
        level += strtol(shell_level, NULL, 10);
    }

    if (level > 1000) {
        log_error("warning: shell level (%d) too high, resetting to 1\n", level);
        level = 1;
    } else if (level < 0) {
        level = 0;
    }

    snprintf(shell_level_str, sizeof(shell_level_str), "%ld", level);
    setenv("SHLVL", shell_level_str, 1);
}

void handle_arguments(int argc, char **argv) {
    if (argc < 2) {
        return;
    }

    const char help_text[] = "csh: usage: csh [--help] [-c] command";

    if (strcmp(argv[1], "--help") == 0) {
        printf(help_text);
        exit(EXIT_SUCCESS);
    }

    // we only accept `-c` as the first argument. If it doesn't have any arguments afterwards, log error
    if (strcmp(argv[1], "-c") != 0 || argc < 3) {
        log_error_with_exit(help_text);
    }

    char *original_input = strdup(argv[2]);
    execute_input(original_input);
    exit(last_exit_code);
}

int main(int argc, char **argv) {
    setup_signal_handlers();
    initialize_shell();
    atexit(cleanup);

    handle_arguments(argc, argv);

    while (continue_execution) {
        char *original_input = cshr_read_input(get_prompt());
        execute_input(original_input);
    }

    return 0;
}
