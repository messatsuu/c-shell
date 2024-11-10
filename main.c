#include "sys/wait.h"
#include "time.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <stdbool.h>

#define MAX_ARGUMENTS_SIZE 100

const char *builtin_commands[] = { "cd" };
// the first sizeof() returns the number of bytes that the array holds. builtin_commands is an array of pointers.
// In a 64-bit system, each pointer is 8 bytes long, meaning we get a total of 16 bytes, divided by the size of one pointer
// (16 / 8) and we have the number of elements
int number_of_builtin_commands = sizeof(builtin_commands) / sizeof(builtin_commands[0]);

bool is_builtin_command(char command[]) {
    for (size_t i = 0; i < number_of_builtin_commands; i++) {
        if (strcmp(command, builtin_commands[i]) == 0) {
            return true;
        }
    }
    return false;
}

int run_builtin_command(char *command[]) {
    if (strcmp("cd", command[0]) == 0) {
        char* path = command[1];
        // If no path provided, use $HOME
        if (path == NULL) {
            path = getenv("HOME");
            if (path == NULL) {
                fprintf(stderr, "cd: HOME environment variable is not set\n");
                return -1;
            }
        }
        chdir(path);
    }

    return 0;
}

// If the command is not a builtin, we run it as a child process
int run_child_process(char *args[]) {
    pid_t pid = fork(); // Create a new child process (e.g. fork) that runs the code from here on

    if (pid < 0) {
        perror("Fork failed");
        return -1;
    }

    // child process (pid 0), execute the command and exit
    if (pid == 0) {
        // Execute the command
        execvp(args[0], args);
        exit(0);
    }

    int status;
    // parent process (pid > 0), wait for child to finish
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid() failed");
        return -1;
    }

    return WEXITSTATUS(status);
}

int execute_command(char *input) {
    char *token = strtok(input, " ");
    char *args[MAX_ARGUMENTS_SIZE];
    args[0] = strdup(token);

    // Fill args with the rest of the input
    int i = 1;
    while ((token = strtok(NULL, " ")) != NULL && i < MAX_ARGUMENTS_SIZE - 1) {
        args[i++] = token;
    }
    // null-terminate the arguments array
    args[i] = NULL;

    if (is_builtin_command(args[0])) {
        run_builtin_command(args);
        return 0;
    }

    int process_exit_status = run_child_process(args);

    if (process_exit_status != 0) {
        return -1;
    }

    return 0;
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

        // Find position of newline, assign with null terminator
        input[strcspn(input, "\n")] = '\0';

        int exit_code = execute_command(input);

        if (exit_code != 0) {
            printf("\n-- command failed --\n");
        }
    }
}
