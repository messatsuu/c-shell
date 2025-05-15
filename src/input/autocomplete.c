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
            strcat(result_string, " ");
        }
    }

    print_under_input(result_string);
}

void set_dir_entries(AutocompleteResult *autocompleteResult, char *path)
{
    DIR * directory = opendir(path); // open the path
    if (directory == NULL) {
        return;
    }

    struct dirent * entry;
    while ((entry = readdir(directory)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        if (autocompleteResult->count >= autocompleteResult->capacity) {
            reallocate_autocomplete_entries(autocompleteResult , BUF_EXPANSION_SIZE);
        }
        add_entry_to_autocomplete_result(autocompleteResult, entry->d_name);
    }
    closedir(directory);
}

void autocomplete_current_path(AutocompleteResult *autocompleteResult) {
    char *current_path = getenv("PWD");
    set_dir_entries(autocompleteResult, current_path);
}

void autocomplete(char *buffer, unsigned long buffer_length) {
    AutocompleteResult autocompleteResult;

    init_autocomplete_result(&autocompleteResult);
    autocomplete_current_path(&autocompleteResult);
    print_autocomplete_entries(&autocompleteResult);
}
