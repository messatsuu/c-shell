#include <parser.h>
#include <process.h>
#include <utility.h>
#include <execvp.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

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
    int previous_pipe_file_read_end = -1;
    char*** commands = create_piped_command_array(args);
    int pids[MAX_COMMANDS] = {0};
    int statuses[MAX_COMMANDS] = {0};
    int status = 0;

    for (; commands[i] != NULL; i++) {
        bool is_last_command = commands[i+1] == NULL;
        if (!is_last_command) {
            // Setup pipe to redirect STDIN/STDOUT
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
            // If a previous pipe is set to read from, duplicate it onto STDIN (and close the original)
            if (previous_pipe_file_read_end != -1) {
                // Make previous read_end the new STDIN
                dup2(previous_pipe_file_read_end, STDIN_FILENO);
                close(previous_pipe_file_read_end);
            }

            // If not the last command, duplicate the pipe_file's write-end onto STDOUT (and close original one)
            // Essentially making the pipe_file's write-end the new STDOUT (the execvp() writes to)
            if (!is_last_command) {
                close(pipe_file_descriptor[0]); // Close read end
                dup2(pipe_file_descriptor[1], STDOUT_FILENO); // Duplicate the pipe_file's write-end onto STDOUT
                close(pipe_file_descriptor[1]);
            }

            // execvp() replaces the process image, 
            run_execvp(commands[i]);
        }

        // Parent process
        if (previous_pipe_file_read_end != -1) close(previous_pipe_file_read_end);
        // If there is a next command in the pipeline, set up the read-end for the next child to read from
        if (!is_last_command) {
            close(pipe_file_descriptor[1]); // Close write end
            previous_pipe_file_read_end = pipe_file_descriptor[0];
        }

        pids[i] = pid;
    }

    // Wait for all children; If any process return a non-zero exit-code, return it
    for (int j = 0; j <= i; j++) {
        waitpid(pids[i], &statuses[i], 0);
        status = WEXITSTATUS(statuses[i]);
        if (status != 0) {
            break;
        }
    }

    // Free all commands allocated by parser
    for (int j = 0; j <= i; j++) {
        free(commands[j]);
    }
    free(commands);


    return status;
}
