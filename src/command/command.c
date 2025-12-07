#include "command/command.h"
#include "ast/ast.h"
#include "core/process.h"
#include <utility.h>

#include <stdbool.h>
#include <unistd.h>

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
            last_exit_code = run_child_process_pipeline_ast(astList->list.pipelines[i]);
        }
    }
}
