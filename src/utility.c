#include <history.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <unistd.h>

void log_error(const char *format, ...) {
    va_list args;
    va_start(args, format);

    if (vfprintf(stderr, format, args) < 0) {
        perror("Error writing to stderr");
    }

    va_end(args);
}

noreturn void log_error_with_exit(const char *message) {
    log_error(message);
    exit(EXIT_FAILURE);
}

void *reallocate(void *pointer, size_t size,  bool exit) {
    void *reallocation_result = realloc(pointer, size);
    if (!reallocation_result) {
        free(pointer);
        if (exit) {
            log_error_with_exit("Reallocation Error");
        }
        return NULL;
    }

    return reallocation_result;
}

void *allocate(size_t size, bool exit) {
    void *allocation_result = malloc(size);
    if (!allocation_result) {
        if (exit) {
            log_error_with_exit("Allocation Error");
        }
        return NULL;
    }

    return allocation_result;
}

void *callocate(unsigned int number_of_bytes, size_t size, bool exit) {
    void *callocation_result = calloc(number_of_bytes, size);
    if (!callocation_result) {
        if (exit) {
            log_error_with_exit("Zero-Value Allocation Error");
        }
        return NULL;
    }

    return callocation_result;
}

void cleanup() {
    cleanup_history();
}

// Wrapper function to override in testing (statically linked glibc-functions cannot be overriden in github actions?)
ssize_t get_host_name(char *name, size_t len) {
    return gethostname(name, len);
}
