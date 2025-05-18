#include <process.h>
#include <utility.h>
#include <parser.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

// TODO: Find a way to run a command that covers the following points:
// - The command should be capturable (pipe, FILE* redirect, etc.) for unit-testing
// - The command should be able to run tui-commands (nvim, less) without providing a pty
int run_execvp(char** args) {
    // Child process
    execvp(args[0], args);
    switch (errno) {
        case ENOENT:
            log_error("Command not found: %s\n", args[0]);
            exit(127);
        case EACCES:
            log_error("Permission denied: %s\n", args[0]);
            exit(126);
        default:
            log_error("Error executing command: %s\n", args[0]);
            perror("error");
            exit(1);
    }
}

int run_child_process_normal(char *args[]) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return -1;
    }

    if (pid == 0) {
        run_execvp(args);
    }

    int status = 0;
    if (waitpid(pid, &status, 0) == -1) {
        perror("Waiting for child process failed");
        return -1;
    }

    return WEXITSTATUS(status);
}

int run_child_process_piped(char *args[]) {
    unsigned int i = 0;
    int pipe_file_descriptor[2];
    // Setting file_descriptor to impossible initial value
    int previous_file_descriptor = -1;
    char*** commands = create_piped_command_array(args);
    int pids[MAX_COMMANDS] = {0};
    int statuses[MAX_COMMANDS] = {0};
    int status = 0;

    for (; commands[i] != NULL; i++) {
        bool is_last_command = commands[i+1] == NULL;
        if (!is_last_command) {
            // Not the last command, so create a pipe
            if (pipe(pipe_file_descriptor) == -1) {
                perror("pipe error");
                log_error_with_exit("pipe could not be created");
            }
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        // Child process
        if (pid == 0) {
            // If not the first command, redirect input
            if (previous_file_descriptor != -1) {
                dup2(previous_file_descriptor, STDIN_FILENO);
                close(previous_file_descriptor);
            }

            // If not the last command, redirect output
            if (!is_last_command) {
                close(pipe_file_descriptor[0]); // Close read end
                dup2(pipe_file_descriptor[1], STDOUT_FILENO);
                close(pipe_file_descriptor[1]);
            }

            // Child exits after this line
            run_execvp(commands[i]);
        }

        // Parent process
        if (previous_file_descriptor != -1) close(previous_file_descriptor);
        if (commands[i+1] != NULL) {
            close(pipe_file_descriptor[1]); // Close write end
            previous_file_descriptor = pipe_file_descriptor[0];
        }

        pids[i] = pid;
    }

    // Wait for all children; If any process return a non-zero exit-code, return it
    for (int j = 0; j <= i; j++) {
        waitpid(pids[i], &statuses[i], 0);
        status = WEXITSTATUS(statuses[i]);
        if (status != 0) {
            return status;
        }
    }

    return 0;
}
