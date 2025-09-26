#include "parser/command_parser.h"
#include "command/command.h"
#include "fcntl.h"
#include "unistd.h"
#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void set_command_flags(Command *command) {
    char *redirect_file_string = strchr(command->command, '>');

    if (strchr(command->command, '|')) {
        command->flags |= CMD_FLAG_PIPE;
    }
    if (redirect_file_string) {
        // If '>>' then APPEND, otherwise REDIRECT
        char *next_char = (redirect_file_string + 1);
        command->flags |= *next_char == '>' ? CMD_FLAG_APPEND : CMD_FLAG_REDIRECT;
    }
    // TODO: implement CMD_FLAG_BACKGROUND
}

void initialize_command(Command *command, char *command_string) {
    command->flags = 0;
    command->separator[0] = '\0';
    command->output_file_descriptor = STDOUT_FILENO;
    command->arguments = (char **)allocate(sizeof(char *) * MAX_ARGS_PER_COMMAND, true);
    command->command = command_string;
}

void cleanup_command(Command *command) {
    free(command->command);

    for (int i = 0; i < command->number_of_arguments; i++) {
        free(command->arguments[i]);
    }

    free((char *)command->arguments);
    if (command->flags & (CMD_FLAG_REDIRECT | CMD_FLAG_APPEND)) {
        close(command->output_file_descriptor);
    }
}

void set_command_string(Command *command) {
    if (command->flags & (CMD_FLAG_REDIRECT | CMD_FLAG_APPEND)) {
        int open_flags = O_WRONLY | O_CREAT | O_TRUNC;
        char *redirect_file_string = strchr(command->command, '>');
        // Terminate command string at position of redirect-char
        command->command[redirect_file_string - command->command] = '\0';

        if (command->flags & CMD_FLAG_APPEND) {
            // Basically the same, besides appending instead of truncating to 0 bytes
            open_flags = O_WRONLY | O_CREAT | O_APPEND;
            redirect_file_string++;
        }

        redirect_file_string++;
        while (*redirect_file_string == ' ') {
            redirect_file_string++;
        }

        int redirect_file_fd = open(redirect_file_string, open_flags, 0644);
        command->output_file_descriptor = redirect_file_fd;
    }
}

void set_command_args(Command *command, char *command_string) {
    char *argument = strtok(command_string, " ");
    unsigned int i = 1;

    command->arguments[0] = strdup(argument);
    while ((argument = strtok(NULL, " ")) != NULL && i < MAX_ARGS_PER_COMMAND) {
        command->arguments[i++] = strdup(argument);
    }

    command->number_of_arguments = i;
}

void convert_input_to_commands(char *input, int *count, Command **commands) {
    int command_capacity = INITIAL_BUFSIZE;

    if (*commands == NULL) {
        *commands = callocate(command_capacity, sizeof(Command), true);
    }

    for (const char *pointer = input; *pointer != '\0';) {
        if (*count >= command_capacity) {
            command_capacity += BUF_EXPANSION_SIZE;
            *commands = reallocate(*commands, command_capacity * sizeof(Command), true);
        }

        // TODO: make it so that `echo "this ; is ; a string"` is a single command
        // TODO: Implement escpaing space separation for arguments (e.g. xdg-open My\ File.zip)
        const char *and_separator = strstr(pointer, "&&");
        const char *or_separator = strstr(pointer, "||");
        const char *semi_separator = strchr(pointer, ';');

        const char *next_separator = NULL;
        const char *selected = NULL;
        const char *separators[] = {and_separator, or_separator, semi_separator};
        const int number_of_separators = sizeof(separators) / sizeof(separators[0]);
        const char *names[] = {"&&", "||", ";"};

        Command *command = &(*commands)[*count];

        // Find the nearest separator
        for (int i = 0; i < number_of_separators; i++) {
            if (separators[i] != NULL && (next_separator == NULL || separators[i] < next_separator)) {
                next_separator = separators[i];
                selected = names[i];
            }
        }

        // If there's a starting whitespace, remove it
        if (*pointer == ' ') {
            pointer++;
        }

        *count = *count + 1;
        char *command_string = !next_separator ? strdup(pointer) : strndup(pointer, next_separator - pointer);
        initialize_command(command, command_string);

        if (!next_separator) {
            // No more separators, grab the rest (until end of string) and finish
            set_command_flags(command);
            set_command_string(command);
            set_command_args(command, command->command);
            break;
        }

        set_command_flags(command);
        set_command_string(command);
        set_command_args(command, command->command);
        strncpy(command->separator, selected, sizeof(command->separator));

        // Continue after the separator
        pointer = next_separator + strlen(selected);
    }
}
