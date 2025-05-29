#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#define MAX_COMMANDS 10
#define MAX_ARGS_PER_COMMAND 10

// 1u == 1 as unsigned integer
#define CMD_FLAG_PIPE       (1u << 0)
#define CMD_FLAG_REDIRECT   (1u << 1) // TODO: implement
#define CMD_FLAG_HISTORY   (1u << 2)
#define CMD_FLAG_BACKGROUND (1u << 3) // TODO: implement

typedef struct {
    char *command;
    char separator[3];
    unsigned int flags;
    int output_file_descriptor;
} Command;

// Convert a string to an array of Command structs
void convert_input_to_commands(char *input, int *count, Command **commands);

#endif
