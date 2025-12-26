#ifndef ALIAS_H
#define ALIAS_H

#define MAX_ALIAS_NAME 255
#define MAX_ALIAS_COMMAND 255

typedef struct {
    const char *name;
    char *command;
} AliasEntry;

typedef struct {
    unsigned int count;
    unsigned int capacity;
    AliasEntry **entries;
} Aliases;

AliasEntry *get_alias_entry(const char *name, unsigned int *index);
AliasEntry *get_alias_entry_recursive(const char *name, unsigned int *index);

int add_alias(char *name, char *command);
int remove_alias(char *name);

void cleanup_aliases();

int print_alias(char *name);
int print_aliases();

#endif
