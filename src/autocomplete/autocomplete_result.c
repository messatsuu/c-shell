#include <stdlib.h>
#include <string.h>
#define _POSIX_C_SOURCE 200809L  // Enables POSIX functions like strdup()

#include <autocomplete.h>
#include <autocomplete_result.h>
#include <terminal.h>
#include <utility.h>
#include <limits.h>

#define BLUE "\033[0;34m"
#define GREEN "\033[32m"
#define WHITE "\033[37m"
#define NORMAL "\033[0m"

void init_autocomplete_result(AutocompleteResult *autocompleteResult) {
    autocompleteResult->entries = allocate(INITIAL_BUFSIZE * sizeof(AutocompleteResultEntry *), true);
    autocompleteResult->count = 0;
    autocompleteResult->capacity = INITIAL_BUFSIZE;
}

void add_entry_to_autocomplete_result(AutocompleteResult *autocompleteResult, char* entry, enum AutocompleteResultEntryType entryType) {
    autocompleteResult->entries[autocompleteResult->count] = allocate(sizeof(AutocompleteResultEntry), true);

    char *new_entry = callocate(ENTRY_MAX, 1, true);
    strncpy(new_entry, entry, strlen(entry));
    switch (entryType) {
        case RESULT_ENTRY_TYPE_DIR: {
            // Add a slash to result entry if type dir
            new_entry[strlen(new_entry)] = '/';
            new_entry[strlen(new_entry)] = '\0';
            break;
        }
        default:
            break;
    }
    autocompleteResult->entries[autocompleteResult->count]->entry = new_entry;
    autocompleteResult->entries[autocompleteResult->count]->resultEntryType = entryType;
    autocompleteResult->count++;
}

int reallocate_autocomplete_entries(AutocompleteResult *autocompleteResult, unsigned int entries_expansion_size) {
    autocompleteResult->capacity += entries_expansion_size;
    autocompleteResult->entries = reallocate(autocompleteResult->entries, autocompleteResult->capacity * sizeof(char *), false);

    if (!autocompleteResult->entries) {
        return -1;
    }

    return 0;
}

void cleanup_autocomplete_result(AutocompleteResult *autocompleteResult) {
    if (autocompleteResult == NULL) {
        return;
    }

    for (int i = 0; i < autocompleteResult->count; i++) {
        free(autocompleteResult->entries[i]->entry);
        free(autocompleteResult->entries[i]);
    }

    free(autocompleteResult->entries);
    free(autocompleteResult->search_term);
}

void print_autocomplete_entries(AutocompleteResult *autocompleteResult) {
    unsigned int buffer_size = INITIAL_BUFSIZE_BIG;
    char *result_buffer = allocate(buffer_size, true);
    result_buffer[0] = '\0';

    for (size_t i = 0; i < autocompleteResult->count; i++) {
        // The Color to output in
        char *output_color = WHITE;
        // additional text to render with autocomplete result
        switch (autocompleteResult->entries[i]->resultEntryType) {
            case RESULT_ENTRY_TYPE_DIR:
                output_color = BLUE;
                break;
            case RESULT_ENTRY_TYPE_FILE:
                output_color = WHITE;
                break;
        }

        // If the size of all elements for the printout is bigger than the current `buffer_size`, reallocate
        if (strlen(result_buffer) + strlen(output_color) + strlen(autocompleteResult->entries[i]->entry) + strlen(NORMAL) + 1 >= buffer_size) {
            buffer_size += BUF_EXPANSION_SIZE_BIG;
            result_buffer = reallocate(result_buffer, buffer_size, true);
        }

        // Put Entry into result-string
        strcat(result_buffer, output_color);
        strcat(result_buffer, autocompleteResult->entries[i]->entry);
        strcat(result_buffer, NORMAL);

        if (i < autocompleteResult->count - 1) {
            // TODO: make it so that this is a certain percent of total screen width, instead of random number of spaces
            strcat(result_buffer, " ");
        }
    }

    print_under_input(result_buffer);
    free(result_buffer);
}

