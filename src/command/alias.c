#include "command/alias.h"
#include "string.h"
#include "unistd.h"
#include "utility.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

Aliases *aliases = nullptr;

#include <stdalign.h>

void init_aliases() {
    aliases = callocate(sizeof(Aliases), 1, true);
    aliases->count = 0;
    aliases->capacity = INITIAL_BUFSIZE;
    aliases->entries = callocate(aliases->capacity, sizeof(AliasEntry *), true);
}

void cleanup_alias(AliasEntry *aliasEntry) {
    // TODO: is the casting here fine?
    free((void *)aliasEntry->name);
    free(aliasEntry->command);
    free(aliasEntry);
}

void cleanup_aliases() {
    if (aliases == nullptr) {
        return;
    }

    for (unsigned int i = 0; i < aliases->count; i++) {
        cleanup_alias(aliases->entries[i]);
    }

    free(aliases->entries);
    free(aliases);
}

int add_alias(char *name, char *command) {
    if (aliases == nullptr) {
        init_aliases();
    }

    if (strchr(name, ' ')) {
        log_error("Alias name \"%s\" cannot contain spaces", name);
        return 1;
    }

    AliasEntry *existingEntry = get_alias_entry(name, nullptr);
    if (existingEntry) {
        free(existingEntry->command);
        existingEntry->command = strdup(command);
        return 0;
    }

    ensure_capacity((void **)&aliases->entries, &aliases->capacity, aliases->count, 1, sizeof(AliasEntry *));

    AliasEntry *aliasEntry = callocate(1, sizeof(AliasEntry), true);
    aliasEntry->name = strdup(name);
    aliasEntry->command = strdup(command);

    aliases->entries[aliases->count++] = aliasEntry;

    return 0;
}

AliasEntry *get_alias_entry(const char *name, unsigned int *index) {
    if (aliases == nullptr) {
        return nullptr;
    }

    for (unsigned int i = 0; i < aliases->count; i++) {
        AliasEntry *aliasEntry = aliases->entries[i];

        if (strcmp(aliasEntry->name, name) == 0) {
            if (index) {
                *index = i;
            }

            return aliasEntry;
        }
    }


    return nullptr;
}

AliasEntry *get_alias_entry_recursive(const char *name, unsigned int *index) {
    AliasEntry *finalEntry = nullptr;
    AliasEntry *aliasEntry = nullptr;

    const char **visited = callocate(INITIAL_BUFSIZE, sizeof(char *), true);
    unsigned int visited_count = 0;

    while ((aliasEntry = get_alias_entry(name, index)) != nullptr) {
        name = aliasEntry->command;

        // If we already traversed this entry, return null
        for (unsigned int i = 0; i < visited_count; i++) {
            if (strcmp(visited[i], name) == 0) {
                free(visited);
                return nullptr;
            }
        }
        visited[visited_count++] = name;
        finalEntry = aliasEntry;
    }

    free(visited);

    return finalEntry;
}

int remove_alias(char *name) {
    if (aliases == nullptr || aliases->count == 0) {
        log_error("Alias-table is empty.");
        return 1;
    }

    unsigned int index = -1;
    AliasEntry *aliasEntry = get_alias_entry(name, &index);
    if (aliasEntry == nullptr) {
        log_error("No alias found with name \"%s\"", name);
        return 1;
    }

    cleanup_alias(aliasEntry);

    // if the removed element is not the last, shift the array one to the left
    if (index != aliases->count - 1) {
        memmove(&aliases->entries[index], &aliases->entries[index + 1], (aliases->count - index - 1) * sizeof(*aliases->entries));
    }

    aliases->count--;
    // clearing dangling pointer
    aliases->entries[aliases->count] = NULL;

    return 0;
}

int clear_aliases() {
    if (aliases == nullptr || aliases->count == 0) {
        log_error("Alias-table is empty.");
        return 1;
    }

    cleanup_aliases();
    init_aliases();
    return 0;
}

int print_alias(char *name) {
    if (aliases == nullptr) {
        log_error("Alias-table is empty.");
        return 1;
    }

    AliasEntry *aliasEntry = get_alias_entry(name, nullptr);

    if (aliasEntry == nullptr) {
        log_error("No alias found with name \"%s\"", name);
        return 1;
    }
    
    printf("%s='%s'\n", aliasEntry->name, aliasEntry->command);

    return 0;
}

int print_aliases() {
    if (aliases == nullptr) {
        log_error("Alias-table is empty.");
        return 1;
    }

    for (unsigned int i = 0; i < aliases->count; i++) {
        AliasEntry *aliasEntry = aliases->entries[i];
        printf("%s='%s'\n", aliasEntry->name, aliasEntry->command);
    }

    return 0;
}

bool expand_aliases(char **input) {
    char *first_space=strchr(*input, ' ');
    unsigned int first_word_length = first_space ? first_space - *input : strlen(*input);
    char *first_word = calloc(1, first_word_length + 1);
    strncpy(first_word, *input, first_word_length);

    AliasEntry *aliasEntry = get_alias_entry_recursive(first_word, nullptr);
    if (aliasEntry == nullptr) {
        free(first_word);
        return false;
    }

    size_t input_length = strlen(*input) + 1;
    int needed = strlen(aliasEntry->command) - strlen(first_word);
    if (needed > 0) {
        // Guarantee enough space in input for replacing alias
        ensure_capacity((void **)input, &input_length, input_length, needed, sizeof(char));
    }

    replace_first_inplace(*input, input_length, first_word, aliasEntry->command);
    free(first_word);

    return true;
}
