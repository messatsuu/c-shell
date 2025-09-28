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
    execvp(argv[0], argv);
    switch (errno) {
        case ENOENT:
            log_error("Command not found: %s\n", argv[0]);
            exit(127);
        case EACCES:
            log_error("Permission denied: %s\n", argv[0]);
            exit(126);
        default:
            log_error("Error executing command: %s\n", argv[0]);
            perror("error");
            exit(1);
    }
}
