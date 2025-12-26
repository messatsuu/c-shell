#include "command/alias.h"
#include "string.h"
#include "utility.h"
#include <stdio.h>
#include <stdlib.h>

Aliases *aliases = nullptr;

void init_aliases() {
    aliases = callocate(sizeof(Aliases), 1, true);
    aliases->count = 0;
    aliases->capacity = INITIAL_BUFSIZE;
    aliases->entries = callocate(INITIAL_BUFSIZE, sizeof(AliasEntry *), true);
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

    AliasEntry *existingEntry = get_alias_entry(name, nullptr, false);
    if (existingEntry) {
        free(existingEntry->command);
        existingEntry->command = strdup(command);
        return 0;
    }

    // reallocate if needed
    if (aliases->count + 1 > aliases->capacity) {
        unsigned int new_capacity = aliases->capacity * 2;
        aliases->entries = reallocate_safe(aliases->entries, aliases->capacity * sizeof(AliasEntry *), new_capacity * sizeof(AliasEntry *), true);
        aliases->capacity = new_capacity;
    }

    AliasEntry *aliasEntry = callocate(1, sizeof(AliasEntry), true);
    aliasEntry->name = strdup(name);
    aliasEntry->command = strdup(command);

    aliases->entries[aliases->count++] = aliasEntry;

    return 0;
}

AliasEntry *get_alias_entry(const char *name, unsigned int *index, bool recursive) {
    if (aliases == nullptr) {
        return nullptr;
    }

    for (unsigned int i = 0; i < aliases->count; i++) {
        AliasEntry *aliasEntry = aliases->entries[i];

        if (strcmp(aliasEntry->name, name) == 0) {
            if (index) {
                *index = i;
            }

            if (!recursive) {
                return aliasEntry;
            }

            AliasEntry *recursiveEntry = get_alias_entry(aliasEntry->command, index, recursive);
            return recursiveEntry != nullptr ? recursiveEntry : aliasEntry ;
        }
    }


    return nullptr;
}

int remove_alias(char *name) {
    if (aliases == nullptr || aliases->count == 0) {
        log_error("Alias-table is empty.");
        return 1;
    }

    unsigned int index = -1;
    AliasEntry *aliasEntry = get_alias_entry(name, &index, false);
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

int print_alias(char *name) {
    if (aliases == nullptr) {
        log_error("Alias-table is empty.");
        return 1;
    }

    AliasEntry *aliasEntry = get_alias_entry(name, nullptr, false);

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
