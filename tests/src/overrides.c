#include "../include/utility.h"

#include "unistd.h"
#include <string.h>

// The original `run_execvp`
int __real_run_execvp(char** args);
// We need to mock `run_execvp()` to put the stdout into a temporary file
int __wrap_run_execvp(char** args) {
    int mock_stdout_file_descriptor = fileno(get_mock_stdout_file());
    // Duplicate `mock_stdout_file`'s FD as the new STDOUT and close the original one
    dup2(mock_stdout_file_descriptor, STDOUT_FILENO);
    close(mock_stdout_file_descriptor);

    return __real_run_execvp(args);
}

// We mock get_host_name() (function in prompt.c), since the host is context-dependant
ssize_t __wrap_get_host_name(char *name, size_t len) {
    strcpy(name, "test-host");
    return 0;
}
