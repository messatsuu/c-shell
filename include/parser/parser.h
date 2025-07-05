#ifndef PARSER_H
#define PARSER_H

#define MAX_COMMANDS 10
#define MAX_ARGS_PER_COMMAND 10

int convert_env_var(char **pointer, char **buffer, unsigned int *buffer_size, unsigned long *index);

// Conversions that mutate the original input that gets persisted in the history. Takes ownership of `input`.
void mutate_original_input(char **input);

// Convert raw input from the user
char *convert_input(char *input);

// Creates an array of ***char split by `|`, e.g.
// {"ls", ".", "|", "grep", "foo"} =>
// {{"ls", "."}, {"grep", "foo"}}
char ***create_piped_command_array(char *args[]);

#endif
