#ifndef PARSER_H
#define PARSER_H

#define MAX_COMMANDS 10

// Conversions that mutate the original input that gets persisted in the history. Takes ownership of `input`.
void mutate_original_input(char **input);

// Convert raw input from the user
void convert_argv(char **argv);

int parse_interpretable_file(char *path);

#endif
