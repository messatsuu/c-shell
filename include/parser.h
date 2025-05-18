#ifndef PARSER_H
#define PARSER_H

#define MAX_COMMANDS 10
#define MAX_ARGS_PER_COMMAND 10

// 1u == 1 as unsigned integer
#define CMD_FLAG_PIPE       (1u << 0)
#define CMD_FLAG_BACKGROUND (1u << 1) // TODO: implement
#define CMD_FLAG_REDIRECT   (1u << 2) // TODO: implement

typedef struct {
    char *command;
    char separator[3];
    unsigned int flags;
} Command;

// Convert an env variable from input to its value and concatenate into `buffer`.
// Return -1 on error, 0 on success
int convert_env_var(char **pointer, char **buffer, unsigned int *buffer_size, unsigned long *index);

// Convert raw input from the user
char *convert_input(char *input);

// Convert a string to an array of Command structs
void convert_input_to_commands(char *input, int *count, Command **commands);

// Creates an array of ***char split by `|`, e.g.
// {"ls", ".", "|", "grep", "foo"} =>
// {{"ls", "."}, {"grep", "foo"}}
char ***create_piped_command_array(char *args[]);

#endif
