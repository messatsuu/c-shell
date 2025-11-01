#include "../include/utility.h"

#include "unistd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// The original `run_execvp`
int __real_run_execvp(char** args);
// We need to mock `run_execvp()` to put the stdout into a temporary file
int __wrap_run_execvp(char** args) {
    struct stat stat;
    fstat(STDOUT_FILENO, &stat);

    // Only redirect the output if the current STDOUT's FD is
    // - not already a pipe             `echo foo | grep bar`
    // - is a tty (not regular file)    `echo foo > bar.txt`
    if (!S_ISFIFO(stat.st_mode) && isatty(STDOUT_FILENO)) {
        int mock_stdout_file_descriptor = fileno(get_mock_stdout_file());
        // Duplicate `mock_stdout_file`'s FD as the new STDOUT
        dup2(mock_stdout_file_descriptor, STDOUT_FILENO);
    }

    return __real_run_execvp(args);
}

// We mock get_host_name() (function in prompt.c), since the host is context-dependant
ssize_t __wrap_get_host_name(char *name, size_t len) {
    strcpy(name, "test-host");
    return 0;
}
