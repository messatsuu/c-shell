#ifndef UTILITY_H
#define UTILITY_H

#include <stdbool.h>
#include <stdio.h>

// Default Variables used across the platform
#define INITIAL_BUFSIZE 24
#define INITIAL_BUFSIZE_BIG 1024
#define BUF_EXPANSION_SIZE 128
#define BUF_EXPANSION_SIZE_BIG 1024
#define MAX_ARGUMENTS_SIZE 100

// Prints a message to stderr
void log_error(const char *format, ...);

// Prints a message to stderr and exits
void log_error_with_exit(const char *message);

// Wrapper around realloc that exits with a message to stderr when realloc fails
void *reallocate(void *pointer, size_t size,  bool exit);

void *reallocate_safe(void *pointer, unsigned int old_size, unsigned int new_size, bool exit);

// Wrapper around malloc that exits with a message to stderr when malloc fails
void *allocate(size_t size, bool exit);

// Wrapper around calloc that exits with a message to stderr when calloc fails
void *callocate(unsigned int number_of_elements, size_t size, bool exit);

// Simple function that calls all other cleanup functions
void cleanup();

// static inline functions are declared in headers...
static inline void ensure_capacity(void **buffer, size_t *capacity, size_t used, size_t needed, size_t element_size) {
    size_t required = used + needed;

    if (required <= *capacity) {
        return;
    }

    size_t new_capacity = *capacity ? *capacity : BUF_EXPANSION_SIZE;
    while (new_capacity < required) {
        if (new_capacity < INITIAL_BUFSIZE_BIG) {
            new_capacity += BUF_EXPANSION_SIZE;
        } else {
            new_capacity *= 2;
        }

        if (new_capacity < *capacity) {
            log_error_with_exit("capacity overflow on reallocation");
        }
    }

    *buffer = reallocate_safe(*buffer, (*capacity) * element_size, new_capacity * element_size, true);
    *capacity = new_capacity;
}


ssize_t get_host_name(char *name, size_t len);

// Replaces the first occurence of a substring in a (writable) heap-allocated string in-place
// returns early if not enough space for the replace-string is considered
void replace_first_inplace(char *string, size_t bufsize, const char *sub_string, const char *replace);

void replace_part_of_string(char *string, size_t bufsize, char *sub_string, size_t sub_len, const char *replace);

#endif
