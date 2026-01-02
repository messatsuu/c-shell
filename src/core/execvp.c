#include <stdio.h>
#include <string.h>
#include <utility.h>

#include "unistd.h"
#include <errno.h>
#include <stdlib.h>

// TODO: Find a way to run a command that covers the following points:
// - The command should be capturable (pipe, FILE* redirect, etc.) for unit-testing
// - The command should be able to run tui-commands (nvim, less) without providing a PTY
int run_execvp(char **argv) {
    // Child process

    // TODO: refactor this
    // - call exec() manually 
    // - build a hash-table for lookups & caching (execvp() scans through $PATH on each call)

    // If first word contains a slash it's a path to a binary
    if (strchr(argv[0], '/')) {
        execve(argv[0], argv, nullptr);
    } else {
        execvp(argv[0], argv);
    }
    switch (errno) {
        case ENOENT:
            log_error("csh: Command not found: %s\n", argv[0]);
            exit(127);
        case EACCES:
            log_error("csh: Permission denied: %s\n", argv[0]);
            exit(126);
        default:
            log_error("csh: Error executing command: %s\n", argv[0]);
            perror("error");
            exit(1);
    }
}
