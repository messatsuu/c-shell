#include <stdio.h>
#include <stdlib.h>
#include "../include/history.h"

void log_error(const char *message) {
    if (fprintf(stderr, "%s\n", message) < 0) {
        perror("Error writing to stderr");
    }
}

void log_error_with_exit(const char *message) {
    log_error(message);
    exit(EXIT_FAILURE);
}

void *reallocate(void *pointer, size_t size) {
    void *reallocation_result = realloc(pointer, size);
    if (!reallocation_result) {
        log_error_with_exit("Reallocation Error");
    }

    return reallocation_result;
}

void cleanup() {
    cleanup_history();
}
