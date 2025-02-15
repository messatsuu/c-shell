#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#define INITIAL_SIZE 1

// Declare a global variable to store history
char **history_array = NULL;
int history_size = 0;

void append_to_history(const char *command) {
    // On first call, we initialize the history-array
    if (history_array == NULL) {
        history_array = malloc(INITIAL_SIZE * sizeof(char*));
        if (!history_array) {
            fprintf(stderr, "Memory allocation error\n");
            exit(1);
        }
    }

    // On subsequent calls, we reallocate the history array to have space for one more element
    if (history_size >= INITIAL_SIZE) {
        history_array = realloc(history_array, (history_size + 1) * sizeof(char*)); // Reallocate memory
        if (!history_array) {
            fprintf(stderr, "Memory allocation error\n");
            exit(1);
        }
    }

    // Allocate memory for the new command and copy it into history
    history_array[history_size] = malloc(strlen(command) + 1); // +1 for null terminator
    if (!history_array[history_size]) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }

    // Copy the command into history
    strncpy(history_array[history_size], command, strlen(command));

    history_size++;
}

void print_history() {
    for (int i = 0; i < history_size; i++) {
        printf("%d  %s\n", i + 1, history_array[i]);
    }
}
