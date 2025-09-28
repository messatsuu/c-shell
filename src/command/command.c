#include "command/command.h"
#include "ast/ast.h"
#include "command/builtins.h"
#include "core/process.h"
#include <stdio.h>
#include <utility.h>

#include <stdbool.h>
#include <unistd.h>

int last_exit_code = 0;

void execute_ast_list(AST *astList) {
    bool should_run = true;

    for (int i = 0; i < astList->list.count; i++) {
        ListType operator = astList->list.operators[i];

        if (operator == LIST_AND) {
            should_run = (last_exit_code == 0);
        } else if (operator == LIST_OR) {
            should_run = (last_exit_code != 0);
        } else if (operator == LIST_SEQ) {
            should_run = true;
        }

        if (should_run) {
            last_exit_code = run_child_process_pipeline_ast(astList->list.pipelines[i]);
        }

    }
}
