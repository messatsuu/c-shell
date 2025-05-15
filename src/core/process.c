#include <process.h>
#include <utility.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

// TODO: Find a way to run a command that covers the following points:
// - The command should be capturable (pipe, FILE* redirect, etc.) for unit-testing
// - The command should be able to run tui-commands (nvim, less) without providing a pty
int run_child_process(char *args[]) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return -1;
    }

    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            switch (errno) {
                case ENOENT:
                    log_error("Command not found: %s\n", args[0]);
                    exit(127);
                case EACCES:
                    log_error("Permission denied: %s\n", args[0]);
                    exit(126);
                default:
                    log_error("Error executing command: %s\n", args[0]);
                    perror("error");
                    exit(1);
            }
        }
    }

    int status = 0;
    if (waitpid(pid, &status, 0) == -1) {
        perror("Waiting for child process failed");
        return -1;
    }

    return WEXITSTATUS(status);
}
