#include "sys/wait.h"
#include "time.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <stdbool.h>

#define MAX_ARGUMENTS_SIZE 100
#define INITIAL_BUFSIZE 20

const char *builtin_commands[] = { "cd", "history", "exit", "last_exit_code" };
// bytes in pointer array divided by nr of bites of a single pointer is the number of pointers
int number_of_builtin_commands = sizeof(builtin_commands) / sizeof(builtin_commands[0]);

int last_exit_code;

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
                return 1;
            }
        }
        if (chdir(path) != 0) {
            perror("error");
            return 1;
        }
    }

    // exit
    if (strcmp("exit", command[0]) == 0) {
        exit(0);
    }

    // last_exit_code
    if (strcmp("last_exit_code", command[0]) == 0) {
        printf("last exit code: %d", last_exit_code);
    }

    return 0;
}

// If the command is not a builtin, we run it as a child process
int run_child_process(char *args[]) {
    // Create a new child process (e.g. fork) that runs the code from here on.
    // PIDs: -1 = fork failed, 0 = child process, >0 = parent process (actual PID)
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return -1;
    }

    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("error");
        }
        exit(EXIT_FAILURE);
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

    // Fill args with the rest of the input and null-terminate it
    int i = 1;
    while ((token = strtok(NULL, " ")) != NULL && i < MAX_ARGUMENTS_SIZE - 1) {
        args[i++] = token;
    }
    args[i] = NULL;

    if (is_builtin_command(args[0])) {
        return run_builtin_command(args);
    }

    return run_child_process(args);
}

char *read_input() {
    size_t buffer_size = INITIAL_BUFSIZE;
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        fprintf(stderr, "Allocation error\n");
        exit(EXIT_FAILURE);
    }

    char *buffer_pointer = buffer;
    size_t length = 0;

    // If fgets gets called a second time (e.g. when buffer size is not sufficient, it reads the rest of the input into the buffer)
    while (fgets(buffer_pointer, buffer_size - length, stdin)) {
        length += strlen(buffer_pointer);
        if (buffer[length - 1] == '\n') { // Complete line read
            buffer[length - 1] = '\0';
            return buffer;
        }

        // Buffer full, add 100 more bytes
        buffer_size += 100;
        buffer = realloc(buffer, buffer_size);
        if (!buffer) {
            fprintf(stderr, "Allocation error\n");
            exit(EXIT_FAILURE);
        }
        buffer_pointer = buffer + length;
    }

    // Handle EOF or error
    if (ferror(stdin)) {
        fprintf(stderr, "Error reading input\n");
        free(buffer);
        return NULL;
    }

    return buffer;
}

int main() {
    while (1) {
        printf("$ ");

        char *input = read_input();
        last_exit_code = execute_command(input);
    }
}
