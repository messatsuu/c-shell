#include "core/shell.h"
#include "core/prompt.h"

#include <cshread/cshread.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utility.h>
#include <wait.h>

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
    // TODO: currently gets executed literally e.g. `echo $PS2` just runs `echo >` with ouput redirection
    // setenv("PS2",  " > ", true);
}

int main() {
    setup_signal_handlers();
    initialize_shell();
    atexit(cleanup);

    while (true) {
        char *original_input = cshr_read_input(get_prompt());
        execute_input(original_input);
    }

    return 0;
}
