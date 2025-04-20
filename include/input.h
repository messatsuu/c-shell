#ifndef INPUT_H
#define INPUT_H

typedef struct {
    char *command;
    char separator[3];
} Command;

// Convert raw input from the user
char *convert_input(const char *input);

// Reads input from the user
char *read_input();

// Convert a string to an array of Command structs
void *convert_input_to_commands(char *input, int *count, Command **commands);

#endif
