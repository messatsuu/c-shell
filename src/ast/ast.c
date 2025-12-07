#include "ast/ast.h"
#include "core/process.h"
#include "sys/wait.h"

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <utility.h>

int last_exit_code = 0;

void execute_ast_list(AST *astList) {
    bool should_run = true;

    for (int i = 0; i < astList->list.pipeline_count; i++) {
        switch (astList->list.operators[i]) {
            case LIST_AND:
                should_run = (last_exit_code == 0);
                break;
            case LIST_OR:
                should_run = (last_exit_code != 0);
            default:
                should_run = true;
        }

        if (should_run) {
            if (!astList->list.is_subshell) {
                last_exit_code = run_child_process_pipeline_ast(astList->list.pipelines[i]);
                continue;
            }

            pid_t pid = fork();
            int status = 0;
            if (pid == -1) {
                perror("fork");
                exit(EXIT_FAILURE);
            }

            // CHILD PROCESS
            if (pid == 0) {
                int exit_code = run_child_process_pipeline_ast(astList->list.pipelines[i]);
                exit(exit_code);
            }

            // PARENT PROCESS
            waitpid(pid, &status, 0);
            status = WEXITSTATUS(status);
        }
    }
}

void cleanup_ast_list(AST *listAst) {
    for (unsigned int i = 0; i < listAst->list.pipeline_count; i++) {
        AST *pipeline = listAst->list.pipelines[i];

        for (unsigned int j = 0; j < pipeline->pipeline.command_count; j++) {
            AST *simpleCommand = pipeline->pipeline.commands[j];

            unsigned int k = 0;
            while (simpleCommand->simple.argv[k] != nullptr) {
                free(simpleCommand->simple.argv[k]);
                k++;
            }

            if (simpleCommand->simple.redirection != nullptr) {
                free(simpleCommand->simple.redirection->redirect_filename);
                free(simpleCommand->simple.redirection);
            }

            free(simpleCommand->simple.argv);
            free(simpleCommand);
        }

        free(pipeline->pipeline.commands);
        free(pipeline);
    }

    free(listAst->list.operators);
    free(listAst->list.pipelines);
    free(listAst);
}
