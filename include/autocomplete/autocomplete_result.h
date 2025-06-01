#ifndef AUTOCOMPLETE_RESULTH_H
#define AUTOCOMPLETE_RESULTH_H

#include "input/input.h"
#define ENTRY_MAX 4096

enum AutocompleteResultEntryType {
    RESULT_ENTRY_TYPE_DIR,
    RESULT_ENTRY_TYPE_FILE
};

typedef struct {
    char *entry;
    enum AutocompleteResultEntryType resultEntryType;
} AutocompleteResultEntry;

typedef struct {
    AutocompleteResultEntry **entries;                 // Array of strings (dynamically allocated)
    unsigned int count;             // Number of entries
    unsigned int capacity;          // Current capacity
    char *search_term;              // The term to filter by
    unsigned int search_term_length; // length of the search-term
} AutocompleteResult;

void init_autocomplete_result(AutocompleteResult *autocompleteResult);

void add_entry_to_autocomplete_result(AutocompleteResult *autocompleteResult, char* entry, enum AutocompleteResultEntryType entryType);

int reallocate_autocomplete_entries(AutocompleteResult *autocompleteResult, unsigned int entries_expansion_size);

void cleanup_autocomplete_result(AutocompleteResult *autocompleteResult);

int sort_autocomplete_result_entries(const void *a, const void *b);

void print_autocomplete_entries(AutocompleteResult *autocompleteResult);

#endif
