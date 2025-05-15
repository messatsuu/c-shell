#define _POSIX_C_SOURCE 200809L  // Enables POSIX functions like strdup()
#define _GNU_SOURCE

#include <autocomplete.h>
#include <terminal.h>

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utility.h>

void init_autocomplete_result(AutocompleteResult *autocompleteResult) {
    autocompleteResult->entries = allocate(INITIAL_BUFSIZE * sizeof(char *), true);
    autocompleteResult->count = 0;
    autocompleteResult->capacity = INITIAL_BUFSIZE;
}

void add_entry_to_autocomplete_result(AutocompleteResult *autocompleteResult, char* entry) {
    autocompleteResult->entries[autocompleteResult->count] = entry;
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
        free(autocompleteResult->entries[i]);
    }

    free(autocompleteResult->entries);
    free(autocompleteResult->search_term);
}

void print_autocomplete_entries(AutocompleteResult *autocompleteResult) {
    unsigned int string_length = 0;
    for (int i = 0; i < autocompleteResult->count; i++) {
        string_length += strlen(autocompleteResult->entries[i]);
    }
    string_length += autocompleteResult->count;
    char *result_string = allocate(string_length, true);
    result_string[0] = '\0';

    for (size_t i = 0; i < autocompleteResult->count; i++) {
        strcat(result_string, autocompleteResult->entries[i]);
        if (i < autocompleteResult->count - 1) {
            // TODO: make it so that this is a certain percent of total screen width, instead of random number of spaces
            strcat(result_string, " ");
        }
    }

    print_under_input(result_string);
    free(result_string);
}

void set_dir_entries(AutocompleteResult *autocompleteResult, char *path)
{
    DIR *directory = opendir(path); // open the path
    if (directory == NULL) {
        return;
    }

    struct dirent * entry;
    while ((entry = readdir(directory)) != NULL)
    {
        // If the entry doesn't start with the search-term or is one of "." or "..", skip
        if (strncmp(entry->d_name, autocompleteResult->search_term, autocompleteResult->search_term_length) != 0 || strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        if (autocompleteResult->count >= autocompleteResult->capacity) {
            reallocate_autocomplete_entries(autocompleteResult , BUF_EXPANSION_SIZE);
        }
        // We need to copy the `d_name`, since `DIR *directory` gets freed after closedir()

        add_entry_to_autocomplete_result(autocompleteResult, strdup(entry->d_name));
    }
    closedir(directory);
}

void autocomplete_current_path(AutocompleteResult *autocompleteResult) {
    char *current_path = getenv("PWD");
    set_dir_entries(autocompleteResult, current_path);
}

void set_seach_term_from_input(InputBuffer *inputBuffer, AutocompleteResult *autocompleteResult) {
    unsigned int word_start_position = inputBuffer->cursor_position;

    // Set the position of the last character before space (start of word)
    for (const char *pointer = &inputBuffer->buffer[inputBuffer->cursor_position - 1]; inputBuffer->cursor_position != 0; pointer--) {
        // Only break if the first character is not already a space
        if (*pointer == ' ' && inputBuffer->cursor_position) {
            break;
        }

        word_start_position--;
    }

    autocompleteResult->search_term_length = (int)inputBuffer->cursor_position - (int)word_start_position;
    autocompleteResult->search_term = allocate(autocompleteResult->search_term_length, true);
    memcpy(autocompleteResult->search_term, &inputBuffer->buffer[word_start_position], autocompleteResult->search_term_length);
}


void autocomplete(InputBuffer *inputBuffer) {
    AutocompleteResult autocompleteResult;

    init_autocomplete_result(&autocompleteResult);
    set_seach_term_from_input(inputBuffer, &autocompleteResult);
    autocomplete_current_path(&autocompleteResult);

    // If we have only a single result, replace the search-term in the inputBuffer with the result
    if (autocompleteResult.count == 1) {
        unsigned int result_length = strlen(autocompleteResult.entries[0]) - autocompleteResult.search_term_length;
        char * result = autocompleteResult.entries[0] + autocompleteResult.search_term_length;
        insert_into_buffer_at_cursor_position(inputBuffer, result, result_length);
    } else if (autocompleteResult.count > 1) {
        print_autocomplete_entries(&autocompleteResult);
    }

    cleanup_autocomplete_result(&autocompleteResult);
}
