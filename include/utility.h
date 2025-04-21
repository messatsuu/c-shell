#ifndef UTILITY_H
#define UTILITY_H

#include <stdio.h>
#include <stdbool.h>

// Prints a message to stderr
void log_error(const char *format, ...);

// Prints a message to stderr and exits
void log_error_with_exit(const char *message);

// Wrapper around realloc that exits with a message to stderr when realloc fails
void *reallocate(void *pointer, size_t size,  bool exit);

// Wrapper around malloc that exits with a message to stderr when realloc fails
void *allocate(size_t size, bool exit);

// Simple function that calls all other cleanup functions
void cleanup();

#endif
