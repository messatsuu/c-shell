#include "time.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <stdbool.h>

#define MAX_ARGUMENTS_SIZE 100

const char *builtin_commands[] = { "cd", "history" };
// the first sizeof() returns the number of bytes that the array holds. builtin_commands is an array of pointers.
// In a 64-bit system, each pointer is 8 bytes long, meaning we get a total of 16 bytes, divided by the size of one pointer
// (16 / 8) and we have the number of elements
int number_of_builtin_commands = sizeof(builtin_commands) / sizeof(builtin_commands[0]);

bool is_builtin_command(char command[]) {
    for (size_t i = 0; i < number_of_builtin_commands; i++) {
        if (strcmp(command, builtin_commands[i]) == 0) {
            return true; // Command is found in the list of built-ins
        }
    }
    return false; // Command not found in the list of built-ins
}

int execute_command(char *input) {
    // Create a new child process (e.g. fork) that runs the code from here on
    pid_t pid = fork();

    char *token = strtok(input, " ");
    // The first token is the command
    char *command = strdup(token);

    if (pid < 0) {
        // Fork failed
        perror("Fork failed");
        return -1;
    }

    // If the current caller is the child process (pid 0), execute the command and exit
    if (pid == 0) {
        if (is_builtin_command(command)) {

        }

        char *args[MAX_ARGUMENTS_SIZE];
        // For whatever reason, execvp wants the first argument to be the command
        args[0] = command;

        // Fill args with the rest of the input
        int i = 1;
        while ((token = strtok(NULL, " ")) != NULL && i < MAX_ARGUMENTS_SIZE - 1) {
            args[i++] = token;
        }
        // null-terminate the arguments array
        args[i] = NULL;

        // Execute the command
        execvp(command, args);
        exit(0);
    } else {
        // In case of it being the parent process, wait for child to finish
        wait(NULL);
    }

    return 1;
}

int main() {
    char input[100];

    while (1) {
        printf("$ ");

        // Handle Error when reading input from stdin
        if (fgets(input, sizeof(input), stdin) == NULL && ferror(stdin)) {
            printf("\nAn error occured reading input");
            continue;
        }

        // Remove newline from input by assigning null terminator at the first occurance of "\n"
        input[strcspn(input, "\n")] = '\0';

        execute_command(input);
    }
}
