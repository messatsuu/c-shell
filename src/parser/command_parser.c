#include "parser/command_parser.h"
#include "fcntl.h"
#include "unistd.h"
#include "utility.h"
#include <stdio.h>
#include <string.h>

void set_command_flags(Command *command) {
    if (strchr(command->command, '|')) {
        command->flags |= CMD_FLAG_PIPE;
    }
    if (strchr(command->command, '>')) {
        command->flags |= CMD_FLAG_REDIRECT;
    }
    // TODO: implement CMD_FLAG_BACKGROUND
}

void initialize_command(Command *command) {
    command->flags = 0;
    command->separator[0] = '\0';
    command->output_file_descriptor = STDOUT_FILENO;
}

void set_command_string(Command *command, char* command_string) {
    char *redirect_file_string = NULL;
    if ((redirect_file_string = strchr(command_string, '>'))) {
        // Terminate command string at position of redirect char
        command_string[redirect_file_string - command_string] = '\0';

        redirect_file_string++;
        while (*redirect_file_string == ' ') {
            redirect_file_string++;
        }

        // TODO: close the FD once command is executed
        int redirect_file_fd = open(redirect_file_string, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        command->output_file_descriptor = redirect_file_fd;
    }

    command->command = command_string;
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
        initialize_command(command);

        if (!next_separator) {
            // No more separators, grab the rest (until end of string) and finish
            set_command_string(command, strdup(pointer));
            set_command_flags(command);
            break;
        }

        set_command_string(command, strndup(pointer, next_separator - pointer));
        set_command_flags(command);
        strncpy(command->separator, selected, sizeof(command->separator));

        // Continue after the separator
        pointer = next_separator + strlen(selected);
    }
}
