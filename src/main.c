#include "core/shell.h"
#include "core/prompt.h"
#include "core/init.h"

#include <cshread/cshread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <utility.h>
#include <wait.h>

extern bool continue_execution;
extern int last_exit_code;

void handle_arguments(int argc, char **argv) {
    if (argc < 2) {
        return;
    }

    const char usage_text[] = "csh: usage: csh [--help] [-c] command";

    if (strcmp(argv[1], "--help") == 0) {
        printf(usage_text);
        exit(EXIT_SUCCESS);
    }

    // we only accept `-c` as the first argument. If it doesn't have any arguments afterwards, log error
    if (strcmp(argv[1], "-c") != 0 || argc < 3) {
        log_error_with_exit(usage_text);
    }

    char *original_input = strdup(argv[2]);
    execute_input(original_input);
    exit(last_exit_code);
}

int main(int argc, char **argv) {
    initialize_shell();
    handle_arguments(argc, argv);

    while (continue_execution) {
        char *original_input = cshr_read_input(get_prompt());
        execute_input(original_input);
    }

    return 0;
}
