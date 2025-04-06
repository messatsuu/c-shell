#include "../include/shell.h"
#include "../include/utility.h"
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <wait.h>

#define MAX_ARGUMENTS_SIZE 100
#define INITIAL_BUFSIZE 20

// On SIGINT, flush terminal input and print PS1
void handle_sigint(int signal) {
    reset_shell();
}

int main() {
    signal(SIGINT, handle_sigint);
    atexit(cleanup);

    while (1) {
        create_prompt();
    }

    return 0;
}
