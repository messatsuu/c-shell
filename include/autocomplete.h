#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

typedef struct {
    char **entries; // Array of strings (dynamically allocated)
    unsigned int count; // Number of entries
    unsigned int capacity; // Current capacity
} AutocompleteResult;

// Reads input from the user
void autocomplete(char *buffer, unsigned long buffer_length);

#endif
