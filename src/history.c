#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include "../include/command.h"

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

    // On subsequent calls, we reallocate the history array so one more char-pointer can be stored
    if (history_size >= INITIAL_SIZE) {
        history_array = realloc(history_array, (history_size + 1) * sizeof(char*));

        if (!history_array) {
            fprintf(stderr, "Memory allocation error\n");
            exit(1);
        }
    }

    // TODO: usting strdup instead of strndup could potentially be unsafe (DoS attack)
    history_array[history_size] = strdup(command);
    if (!history_array[history_size]) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }

    history_size++;
}

int execute_command_from_history(const unsigned int index) {
    if (index >= history_size) {
        printf("History index %i out of range", index);
        return 1;
    }

    unsigned int history_index = index;
    history_index--;

    char *command_from_history = history_array[history_index];

    // we need to Duplicate the string, since making operations on it would change it in the history
    return execute_command(strdup(command_from_history));
}

void print_history() {
    for (int i = 0; i < history_size; i++) {
        printf("%d  %s\n", i + 1, history_array[i]);
    }
}
