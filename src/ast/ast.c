#include "ast/ast.h"

#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <utility.h>

void cleanup_node_type(AST *astNode);

void cleanup_simple(AST *simpleAst) {
    unsigned int i = 0;
    while (simpleAst->simple.argv[i] != nullptr) {
        free(simpleAst->simple.argv[i]);
        i++;
    }

    if (simpleAst->simple.redirection != nullptr) {
        free(simpleAst->simple.redirection->redirect_filename);
        free(simpleAst->simple.redirection);
    }

    free(simpleAst->simple.argv);
    free(simpleAst);
}

void cleanup_subshell(AST *subshellAst) {
    cleanup_node_type(subshellAst->subshell.list);
    free(subshellAst);
}

void cleanup_pipeline(AST *pipelineAst) {
    for (unsigned int i = 0; i < pipelineAst->pipeline.command_count; i++) {
        cleanup_node_type(pipelineAst->pipeline.commands[i]);
    }

    free(pipelineAst->pipeline.commands);
    free(pipelineAst);
}

void cleanup_list(AST *listAst) {
    for (unsigned int i = 0; i < listAst->list.pipeline_count; i++) {
        cleanup_node_type(listAst->list.pipelines[i]);
    }

    free(listAst->list.operators);
    free(listAst->list.pipelines);
    free(listAst);
}

void cleanup_node_type(AST *astNode) {
    switch (astNode->type) {
        case NODE_LIST:
            cleanup_list(astNode);
            break;
        case NODE_PIPELINE:
            cleanup_pipeline(astNode);
            break;
        case NODE_SUBSHELL:
            // TODO: change this once subshell has its own allocated members
            cleanup_subshell(astNode);
            break;
        case NODE_SIMPLE:
            cleanup_simple(astNode);
            break;
    }
}
