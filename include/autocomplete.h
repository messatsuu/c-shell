#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

#include <input.h>

typedef struct {
    char **entries;                 // Array of strings (dynamically allocated)
    unsigned int count;             // Number of entries
    unsigned int capacity;          // Current capacity
    char *search_term;              // The term to filter by
    unsigned int search_term_length; // length of the search-term
} AutocompleteResult;

// Reads input from the user
void autocomplete(InputBuffer *inputBuffer);

#endif
