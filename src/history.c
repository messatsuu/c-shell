#define _POSIX_C_SOURCE 200809L  // Enables POSIX functions like strdup()

#include "../include/command.h"
#include "../include/utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define HISTORY_BUFFER_SIZE 1

typedef struct {
    char **entries; // Array of strings (dynamically allocated)
    unsigned int count;      // Number of current entries
    unsigned int capacity;   // Number of entries until reallocation is needed
} History;

// Declare a global variable to store history
History *history = NULL;

int history_index = 0;

void init_history(int initial_capacity) {
    history = malloc(sizeof(History));
    history->entries = malloc(initial_capacity * sizeof(char *));
    history->count = 0;
    history->capacity = initial_capacity;

    if (!history->entries) {
        log_error_with_exit("History Allocation Error");
    }
}

void cleanup_history() {
    if (history == NULL) {
        return;
    }

    for (int i = 0; i < history->count; i++) {
        free(history->entries[i]);
    }

    free(history->entries);
    free(history);
}

void append_to_history(const char *command) {
    if (history == NULL) {
        // On first call, we initialize the history-struct
        init_history(HISTORY_BUFFER_SIZE);
    } else if (strcmp(history->entries[history->count - 1], command) == 0) {
        // Do not append to history if the previous entry is the same as the current one
        return;
    }

    // If we reach the max capacity, reallocate the the entries buffer
    if (history->count >= history->capacity) {
        history->capacity += HISTORY_BUFFER_SIZE;
        history->entries = reallocate(history->entries, (history->capacity) * sizeof(char*), true);
    }

    // TODO: instead of using strdup, make sure that the original command is not
    history->entries[history->count] = strdup(command);
    if (!history->entries[history->count]) {
        log_error_with_exit("Copying command string to buffer failed");
    }

    history->count++;
}

int execute_command_from_history(const unsigned long index) {
    if (history == NULL || index > history->count || index == 0) {
        printf("History index %lu out of range\n", index);
        return 1;
    }

    unsigned int history_index = index;
    history_index--;

    char *command_from_history = history->entries[history_index];

    // we need to Duplicate the string, since making operations on it would change it in the history
    char* command = strdup(command_from_history);
    int exit_code = execute_command(command);

    free(command);
    return exit_code;
}

char *get_entry_from_history(const unsigned int index) {
    return history->entries[index];
}

void print_history() {
    // If the history hasn't been initialized, return
    if (history == NULL) {
        return;
    }

    for (int i = 0; i < history->count; i++) {
        printf("%d  %s\n", i + 1, history->entries[i]);
    }
}
