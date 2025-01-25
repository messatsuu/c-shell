#include "../include/process.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int run_child_process(char *args[]) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return -1;
    }

    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("error");
            exit(EXIT_FAILURE);
        }
    }

    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid() failed");
        return -1;
    }

    return WEXITSTATUS(status);
}
