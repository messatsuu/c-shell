#ifndef PARSER_H
#define PARSER_H

#define MAX_COMMANDS 10
#define MAX_ARGS_PER_COMMAND 10

// Convert an env variable from input to its value and concatenate into `buffer`.
// Return -1 on error, 0 on success
int convert_env_var(char **pointer, char **buffer, unsigned int *buffer_size, unsigned long *index);

// Convert raw input from the user
char *convert_input(char *input);

// Creates an array of ***char split by `|`, e.g.
// {"ls", ".", "|", "grep", "foo"} =>
// {{"ls", "."}, {"grep", "foo"}}
char ***create_piped_command_array(char *args[]);

#endif
