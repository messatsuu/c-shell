#include "ast/ast.h"
#include "ast/ast_executor.h"
#include "command/builtins.h"
#include "core/execvp.h"

#include "fcntl.h"
#include "parser/parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <utility.h>

int last_exit_code = 0;

void execute_node_type(AST *astNode);

void redirect_to_file_descriptor(int *redirect_fd, int *saved_target_fd, int *overriden_fd, Redirection *redirection) {
    int file_mode = 0;
    int target_fd = -1;

    switch (redirection->type) {
        case REDIR_OUT:
            file_mode = O_WRONLY | O_CREAT | O_TRUNC;
            target_fd = STDOUT_FILENO;
            break;
        case REDIR_OUT_APP:
            file_mode = O_WRONLY | O_CREAT | O_APPEND;
            target_fd = STDOUT_FILENO;
            break;
        case REDIR_IN:
            file_mode = O_RDONLY;
            target_fd = STDIN_FILENO;
    }

    *redirect_fd = open(redirection->redirect_filename, file_mode, 0644);

    if (saved_target_fd != nullptr) {
        *saved_target_fd = dup(target_fd);
    }
    if (overriden_fd != nullptr) {
        *overriden_fd = target_fd;
    }

    // Duplicate the redirect_fd onto STDOUT / STDIN
    if (dup2(*redirect_fd, target_fd) < 0) perror("dup2");
}

int execute_simple(AST *simpleAst, bool is_last_command, int *previous_pipe_file_read_end) {
    int pipe_file_descriptor[2];
    int redirect_fd = -1;
    convert_argv(simpleAst->simple.argv);

    if (!is_last_command) {
        // Setup pipe to redirect STDIN/STDOUT
        if (pipe(pipe_file_descriptor) == -1) {
            perror("pipe error");
            log_error_with_exit("pipe could not be created");
        }
    } else if (is_builtin_command(simpleAst->simple.argv[0]) && *previous_pipe_file_read_end == -1) {
        if (!simpleAst->simple.redirection) {
            return run_builtin_command(simpleAst->simple.argv);
        }

        int saved_target_fd = -1;
        int overriden_fd = -1;
        int exit_code = 0;
        // duplicate file's FD to STDOUT/STDIN
        redirect_to_file_descriptor(&redirect_fd, &saved_target_fd, &overriden_fd, simpleAst->simple.redirection);

        exit_code = run_builtin_command(simpleAst->simple.argv);
        fflush(stdout);

        close(redirect_fd);
        // TODO: valgrind reports that STDOUT isn't closed properly since we modified it?
        dup2(saved_target_fd, overriden_fd);
        close(saved_target_fd);

        return exit_code;
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    // CHILD PROCESS
    if (pid == 0) {
        // If a previous pipe's read-end is set to read from, duplicate it onto STDIN (and close the original)
        if (*previous_pipe_file_read_end != -1) {
            // Make previous read_end the new STDIN
            dup2(*previous_pipe_file_read_end, STDIN_FILENO);
            close(*previous_pipe_file_read_end);
        }

        // If not the last command, duplicate the pipe_file's write-end onto STDOUT (and close original one)
        // Essentially making the pipe_file's write-end the new STDOUT (that execvp()/builtin writes to)
        if (!is_last_command) {
            close(pipe_file_descriptor[0]); // Close read end
            dup2(pipe_file_descriptor[1], STDOUT_FILENO); 
            close(pipe_file_descriptor[1]);
        } else if (simpleAst->simple.redirection != nullptr) {
            redirect_to_file_descriptor(&redirect_fd, nullptr, nullptr, simpleAst->simple.redirection);
        }

        // Builtins can run in child processes if they are piped
        if (is_builtin_command(simpleAst->simple.argv[0])) {
            unsigned int exit_code = run_builtin_command(simpleAst->simple.argv);
            _exit(exit_code);
        } else {
            run_execvp(simpleAst->simple.argv);
            _exit(127); // if this line is reached, execvp failed
        }

        // TODO: does this close correctly in the child-process context?
        if (redirect_fd != -1) {
            close(redirect_fd);
        }
    }

    // PARENT PROCESS
    if (*previous_pipe_file_read_end != -1) {
        close(*previous_pipe_file_read_end);
    }
    // If there is a next command in the pipeline, set up the read-end for the next child to read from
    if (!is_last_command) {
        close(pipe_file_descriptor[1]); // Close write end
        *previous_pipe_file_read_end = pipe_file_descriptor[0];
    }

    return pid;
}

int execute_subshell(AST *subshellAst, bool is_last_command, int *previous_pipe_file_read_end) {
    int pipe_file_descriptor[2];

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

    // CHILD PROCESS
    if (pid == 0) {
        // If a previous pipe's read-end is set to read from, duplicate it onto STDIN (and close the original)
        if (*previous_pipe_file_read_end != -1) {
            // Make previous read_end the new STDIN
            dup2(*previous_pipe_file_read_end, STDIN_FILENO);
            close(*previous_pipe_file_read_end);
        }

        // If not the last command, duplicate the pipe_file's write-end onto STDOUT (and close original one)
        // Essentially making the pipe_file's write-end the new STDOUT (the execvp() writes to)
        if (!is_last_command) {
            close(pipe_file_descriptor[0]); // Close read end
            dup2(pipe_file_descriptor[1], STDOUT_FILENO);
            close(pipe_file_descriptor[1]);
        }
        // TODO: subshells can also redirect output; implement here

        execute_node_type(subshellAst->subshell.list);
        _exit(last_exit_code);
    }

    // PARENT PROCESS
    if (*previous_pipe_file_read_end != -1) {
        close(*previous_pipe_file_read_end);
    }
    // If there is a next command in the pipeline, set up the read-end for the next child to read from
    if (!is_last_command) {
        close(pipe_file_descriptor[1]); // Close write end
        *previous_pipe_file_read_end = pipe_file_descriptor[0];
    }

    return pid;
}

void execute_pipeline(AST *pipelineAst) {
    int number_of_commands = pipelineAst->pipeline.command_count;
    // Setting file_descriptor to impossible initial value
    int previous_pipe_file_read_end = -1;
    int *pids = callocate(number_of_commands, sizeof(int), true);
    int *statuses = callocate(number_of_commands, sizeof(int), true);

    for (unsigned int i = 0; i < number_of_commands; i++) {
        bool is_last_command = i + 1 == number_of_commands;
        AST *command = pipelineAst->pipeline.commands[i];

        int pid = -1;

        if (command->type == NODE_SUBSHELL) {
            pid = execute_subshell(command, is_last_command, &previous_pipe_file_read_end);
        } else {
            pid = execute_simple(command, is_last_command, &previous_pipe_file_read_end);
            // -1 signals no fork, so we return early
            if (pid == -1) {
                free(statuses);
                free(pids);
                return;
            }
        }

        pids[i] = pid;
    }

    // Wait for all children; return the exit-code of the last process.
    int last_status = 0;
    for (int j = 0; j < number_of_commands; j++) {
        waitpid(pids[j], &statuses[j], 0);

        int current_status = 0;

        if (WIFEXITED(statuses[j])) {
            current_status = WEXITSTATUS(statuses[j]);
        } else if (WIFSIGNALED(statuses[j])) {
            current_status = 128 + WTERMSIG(statuses[j]);
        } else {
            current_status = -1;
        }

        // Track status of last command only
        if (j == number_of_commands - 1) {
            last_status = current_status;
        }
    }

    free(statuses);
    free(pids);

    last_exit_code = last_status;
}

void execute_list(AST *astList) {
    bool should_run = true;

    for (int i = 0; i < astList->list.pipeline_count; i++) {
        switch (astList->list.operators[i]) {
            case LIST_AND:
                should_run = (last_exit_code == 0);
                break;
            case LIST_OR:
                should_run = (last_exit_code != 0);
                break;
            default:
                should_run = true;
        }

        if (should_run) {
            execute_node_type(astList->list.pipelines[i]);
        }
    }
}

void execute_node_type(AST *astNode) {
    switch (astNode->type) {
        case NODE_LIST:
            execute_list(astNode);
            break;
        case NODE_PIPELINE:
            execute_pipeline(astNode);
            break;
        case NODE_SUBSHELL:
            execute_list(astNode);
            break;
        case NODE_SIMPLE:
            log_error("SIMPLE-node only be called via direct context");
            break;
    }
}
