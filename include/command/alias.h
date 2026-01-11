#ifndef ALIAS_H
#define ALIAS_H

#include <stddef.h>

#define MAX_ALIAS_NAME 255
#define MAX_ALIAS_COMMAND 255

typedef struct {
    const char *name;
    char *command;
} AliasEntry;

typedef struct {
    unsigned long count; // number of used AliasEntrys
    unsigned long capacity; // number of allocated AliasEntrys
    AliasEntry **entries;
} Aliases;

AliasEntry *get_alias_entry(const char *name, unsigned int *index);
AliasEntry *get_alias_entry_recursive(const char *name, unsigned int *index);

int add_alias(char *name, char *command);
int remove_alias(char *name);
int clear_aliases();

void cleanup_aliases();

int print_alias(char *name);
int print_aliases();
// replace aliases in string with minimal context-parsing
void expand_aliases(char **input);

#endif
