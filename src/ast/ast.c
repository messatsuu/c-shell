#include "ast/ast.h"
#include <stdlib.h>

void cleanup_ast_list(AST *listAst) {
    for (unsigned int i = 0; i < listAst->list.count; i++) {
        AST *pipeline = listAst->list.pipelines[i];

        for (unsigned int j = 0; j < pipeline->pipeline.count; j++) {
            AST *simpleCommand = pipeline->pipeline.commands[j];

            unsigned int k = 0;
            while (simpleCommand->simple.argv[k] != nullptr) {
                free(simpleCommand->simple.argv[k]);
                k++;
            }

            if (simpleCommand->simple.redirection != nullptr) {
                fclose(simpleCommand->simple.redirection->redirect_file);
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
