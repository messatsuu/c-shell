#ifndef PARSER_H
#define PARSER_H

typedef struct {
    char *command;
    char separator[3];
} Command;

// Convert an env variable from input to its value and concatenate into `buffer`.
// Return -1 on error, 0 on success
int convert_env_var(char **pointer, char **buffer, unsigned int *buffer_size, unsigned long *index);

// Convert raw input from the user
char *convert_input(const char *input);

// Convert a string to an array of Command structs
void *convert_input_to_commands(char *input, int *count, Command **commands);

#endif
