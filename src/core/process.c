#include "command/builtins.h"
#include "core/execvp.h"
#include "core/process.h"
#include "parser/parser.h"

#include <utility.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int run_child_process_pipeline_ast(AST *pipeline) {
    int pipe_file_descriptor[2];
    int number_of_commands = pipeline->pipeline.count;
    // Setting file_descriptor to impossible initial value
    int previous_pipe_file_read_end = -1;
    int *pids = callocate(number_of_commands, sizeof(int), true);
    int *statuses = callocate(number_of_commands, sizeof(int), true);
    int status = 0;

    for (unsigned int i = 0; i < number_of_commands; i++) {
        bool is_last_command = i + 1 == number_of_commands;
        AST *simpleCommand = pipeline->pipeline.commands[i];
        convert_argv(simpleCommand->simple.argv);

        if (!is_last_command) {
            // Setup pipe to redirect STDIN/STDOUT
            if (pipe(pipe_file_descriptor) == -1) {
                perror("pipe error");
                log_error_with_exit("pipe could not be created");
            }
        } else if (is_builtin_command(simpleCommand->simple.argv[0])) {
            // If the command is a builtin and doesn't pipe, we run it without forking
            run_builtin_command(simpleCommand->simple.argv);
            continue;
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        }

        // CHILD PROCESS
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
            // } else if (simpleCommand->simple.redirection) {
                // TODO: implement!
                // If it is the last command and it should be redirected to another file-descriptor, clone it onto STDOUT
                // dup2(command->output_file_descriptor, STDOUT_FILENO);
            }

            // Builtins can run in child processes if they are piped
            if (is_builtin_command(simpleCommand->simple.argv[0])) {
                run_builtin_command(simpleCommand->simple.argv);
            } else {
                run_execvp(simpleCommand->simple.argv);
            }
        }

        // PARENT PROCESS
        if (previous_pipe_file_read_end != -1) {
            close(previous_pipe_file_read_end);
        }
        // If there is a next command in the pipeline, set up the read-end for the next child to read from
        if (!is_last_command) {
            close(pipe_file_descriptor[1]); // Close write end
            previous_pipe_file_read_end = pipe_file_descriptor[0];
        }

        pids[i] = pid;
    }

    // Wait for all children; If any process return a non-zero exit-code, return it
    for (int j = 0; j < number_of_commands; j++) {
        waitpid(pids[j], &statuses[j], 0);
        status = WEXITSTATUS(statuses[j]);
        if (status != 0) {
            break;
        }
    }

    free(statuses);
    free(pids);

    return status;
}
