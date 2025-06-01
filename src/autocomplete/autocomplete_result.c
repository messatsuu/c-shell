#include <stdlib.h>
#include <string.h>

#include "autocomplete/autocomplete.h"
#include "autocomplete/autocomplete_result.h"
#include "core/terminal.h"
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
        case RESULT_ENTRY_TYPE_FILE: {
            // Add a space to result entry if type file
            new_entry[strlen(new_entry)] = ' ';
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

int get_longest_autocomplete_result_length(AutocompleteResult *autocompleteResult) {
    unsigned int longest_result_length = 0;

    for (int i = 0; i < autocompleteResult->count; i++) {
        unsigned int autocomplete_result_length = strlen(autocompleteResult->entries[i]->entry);
        if (autocomplete_result_length > longest_result_length) {
            longest_result_length = autocomplete_result_length;
        }
    }

    return longest_result_length;
}

void print_autocomplete_entries(AutocompleteResult *autocompleteResult) {
    if (autocompleteResult->count == 0) {
        return;
    }

    unsigned int buffer_size = INITIAL_BUFSIZE_BIG;
    char *result_buffer = callocate(buffer_size, 1, true);

    unsigned terminal_column_count = get_terminal_columns_count();
    unsigned int longest_result_length = get_longest_autocomplete_result_length(autocompleteResult);

    // If there aren't that many results, we don't want to use the whole screen width
    // TODO: might cause issues with longer file-names
    if (autocompleteResult->count < (terminal_column_count / 2)) {
        terminal_column_count = terminal_column_count / 2;
    }

    unsigned int entries_per_row = terminal_column_count / longest_result_length;
    unsigned int spaces_per_row = (terminal_column_count - (longest_result_length * entries_per_row)) / entries_per_row;

    if (entries_per_row == 0) {
        entries_per_row = 1;
        spaces_per_row = 1;
    } else {
        while (spaces_per_row == 0 && entries_per_row > 1) {
            entries_per_row--;
            spaces_per_row = (terminal_column_count - (longest_result_length * entries_per_row)) / entries_per_row;
        }
    }

    // using memcpy with manually tracked index to avoid using strcat (slow due to rescan of whole buffer)
    unsigned int index = 0;

    for (size_t i = 0; i < autocompleteResult->count; i++) {
        // The Color to output in
        char *output_color = WHITE;
        char *current_autocomplete_result = autocompleteResult->entries[i]->entry;
        unsigned int spaces_to_print = longest_result_length - strlen(current_autocomplete_result) + spaces_per_row;

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
        if (strlen(result_buffer) + strlen(output_color) + strlen(current_autocomplete_result) + strlen(NORMAL) + spaces_to_print + 1 >= buffer_size) {
            buffer_size += BUF_EXPANSION_SIZE_BIG;
            result_buffer = reallocate(result_buffer, buffer_size, true);
        }

        // Put Entry into result-string
        memcpy(result_buffer + index, output_color, strlen(output_color));
        index += strlen(output_color);
        memcpy(result_buffer + index, current_autocomplete_result, strlen(current_autocomplete_result));
        index += strlen(current_autocomplete_result);
        memcpy(result_buffer + index, NORMAL, strlen(NORMAL));
        index += strlen(NORMAL);

        if (i < autocompleteResult->count - 1) {
            for (int j = 0; j < spaces_to_print; j++) {
                memcpy(result_buffer + index, " ", 1);
                index++;
            }

            if (((i + 1) % entries_per_row) == 0) {
                char *newline_sequence = "\n\r\x1b[2K";
                memcpy(result_buffer + index, newline_sequence, strlen(newline_sequence));
                index += strlen(newline_sequence);
            }
        }
    }

    result_buffer[index] = '\0';

    print_under_input(result_buffer);
    free(result_buffer);
}

