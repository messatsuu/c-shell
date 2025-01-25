#include <unistd.h>
#include <wait.h>
#include <stdbool.h>
#include "../include/shell.h"

#define MAX_ARGUMENTS_SIZE 100
#define INITIAL_BUFSIZE 20

// On SIGINT, flush terminal input and print PS1
void handle_sigint(int signal) {
    reset_shell();
}

int main() {
    signal(SIGINT, handle_sigint);
    create_shell();
    return 0;
}
