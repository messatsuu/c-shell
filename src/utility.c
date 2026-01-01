// #include "input/history.h"
#include "utility.h"
#include "command/alias.h"

#include <cshread/cshread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>
#include <unistd.h>

// Tell the compiler to ignore [-Wformat-nonliteral] on vfprintf-call (disable literal-checking)
// 1 = position of the format-string parameter
// 2 = position of the first variadic argument
__attribute__((format(printf, 1, 2)))
void log_error(const char *format, ...) {
    va_list args;
    va_start(args, format);

    // NOTE: Can this cause issues if the called function did not write to perror?
    if (vfprintf(stderr, format, args) < 0) {
        perror("Error writing to stderr");
    }

    if (fprintf(stderr, "\n") < 0) {
        perror("Error writing to stderr");
    }

    va_end(args);
}

noreturn void log_error_with_exit(const char *message) {
    log_error("%s", message);
    exit(EXIT_FAILURE);
}

void *reallocate(void *pointer, size_t size, bool exit) {
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

// Calls realloc() and initializes the added memory with calloc()
void *reallocate_safe(void *pointer, unsigned int old_size, unsigned int new_size, bool exit) {
    void *reallocation_result = realloc(pointer, new_size);
    if (!reallocation_result) {
        free(pointer);
        if (exit) {
            log_error_with_exit("Reallocation Error");
        } else {
            log_error_with_exit("Reallocation Error");
        }
        return NULL;
    }

    if (new_size > old_size) {
        memset((char *)reallocation_result + old_size, 0, new_size - old_size);
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

void *callocate(unsigned int number_of_elements, size_t size, bool exit) {
    void *callocation_result = calloc(number_of_elements, size);
    if (!callocation_result) {
        if (exit) {
            log_error_with_exit("Zero-Value Allocation Error");
        }
        return NULL;
    }

    return callocation_result;
}

void cleanup() {
    cshr_history_cleanup();
    cleanup_aliases();
}

// Wrapper function to override in testing (statically linked glibc-functions cannot be overriden in github actions?)
ssize_t get_host_name(char *name, size_t len) {
    return gethostname(name, len);
}

void replace_first_inplace(char *string, size_t bufsize, const char *sub_string, const char *replace) {
    char *sub_string_position = strstr(string, sub_string);
    if (!sub_string_position) {
        return;
    }

    unsigned int sub_string_length = strlen(sub_string);
    unsigned int replace_length = strlen(replace);
    unsigned int after_sub_string_length = strlen(sub_string_position + sub_string_length);

    // return if we don't have enough space
    if (replace_length > sub_string_length && strlen(string) + (replace_length - sub_string_length) + 1 > bufsize) {
        return;
    }

    memmove(sub_string_position + replace_length, sub_string_position + sub_string_length, after_sub_string_length + 1);
    memcpy(sub_string_position, replace, replace_length);
}
