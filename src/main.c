#define _POSIX_C_SOURCE 200809L  // Enables POSIX functions like strdup()

#include "../include/shell.h"
#include "../include/utility.h"
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

#define MAX_ARGUMENTS_SIZE 100
#define INITIAL_BUFSIZE 20

// On SIGINT, flush terminal input and print PS1
void handle_sigint(int signal) {
    reset_shell();
}

void setup_signal_handlers() {
    struct sigaction sig_action;
    sig_action.sa_handler = handle_sigint;
    sigemptyset(&sig_action.sa_mask);

    if (sigaction(SIGINT, &sig_action, NULL) == -1) {
        perror("sigaction");
    }
}

int main() {
    setup_signal_handlers();
    atexit(cleanup);

    while (1) {
        create_prompt();
    }

    return 0;
}
