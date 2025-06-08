#ifndef UTILITY_H
#define UTILITY_H

#include <stdbool.h>
#include <stdio.h>

// Default Variables used across the platform
#define INITIAL_BUFSIZE 20
#define INITIAL_BUFSIZE_BIG 1024
#define BUF_EXPANSION_SIZE 100
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
void *callocate(unsigned int number_of_bytes, size_t size, bool exit);

// Simple function that calls all other cleanup functions
void cleanup();

ssize_t get_host_name(char *name, size_t len);

#endif
