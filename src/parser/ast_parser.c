#include "ast/ast.h"
#include "tokenizer/tokenizer.h"
#include "utility.h"
#include <stdio.h>
#include <string.h>
#include "parser/ast_parser.h"

AST *convert_simple_command(Token **tokens) {
    AST *simpleCommandAST = allocate(sizeof(AST), true);
    simpleCommandAST->type = NODE_SIMPLE;

    simpleCommandAST->simple.argv = (char **)allocate(INITIAL_BUFSIZE * sizeof(char **), true);
    unsigned int i = 0;

    while ((*tokens)->type == TOKEN_WORD) {
        simpleCommandAST->simple.argv[i++] = strdup((*tokens)->text);
        (*tokens)++;
    }

    simpleCommandAST->simple.argv[i++] = nullptr;
    return simpleCommandAST;
}

AST *convert_pipeline(Token **tokens) {
    AST *pipelineAst = allocate(sizeof(AST), true);
    pipelineAst->type = NODE_PIPELINE;
    pipelineAst->pipeline.count = 0;
    pipelineAst->pipeline.commands = (AST **)allocate(INITIAL_BUFSIZE * sizeof(AST *), true);

    while ((*tokens)->type == TOKEN_WORD) {
        pipelineAst->pipeline.commands[pipelineAst->pipeline.count++] = convert_simple_command(tokens);

        if ((*tokens)->type == TOKEN_PIPE) {
            (*tokens)++;
        } else if ((*tokens)->type == TOKEN_OPERAND) {
            break;
        }
    }

    return pipelineAst;
}

void debug_print_ast(AST *listAst) {
    printf("AST PARSING:\n");
    printf("List:\n");
    for (unsigned int i = 0; i < listAst->list.count; i++) {
        printf("pipeline %d:\n", i);
        AST *pipeline = listAst->list.pipelines[i];
        printf("\tcount: %d\n", pipeline->pipeline.count);
        printf("\toperator: %d\n", listAst->list.operators[i]);

        for (unsigned int j = 0; j < pipeline->pipeline.count; j++) {
            printf("\tsimpleCommand %d:\n", j);
            AST *simpleCommand = pipeline->pipeline.commands[j];

            unsigned int k = 0;
            while (simpleCommand->simple.argv[k] != nullptr) {
                printf("\t\targ[%d]: %s\n", k, simpleCommand->simple.argv[k]);
                k++;
            }
        }
    }
}

AST *convert_tokens_to_ast(Token **tokens) {
    AST *listAst = allocate(sizeof(AST), true);
    listAst->type = NODE_LIST;
    listAst->list.operators = allocate(INITIAL_BUFSIZE * sizeof(ListType *), true);
    listAst->list.pipelines = (AST **)allocate(INITIAL_BUFSIZE * sizeof(AST *), true);
    listAst->list.count = 0;

    do {
        ListType list_type = LIST_NONE;

        // TODO: make this better with mapping-array
        if (strcmp("&&", (*tokens)->text) == 0) {
            list_type = LIST_AND;
        } else if (strcmp("||", (*tokens)->text) == 0) {
            list_type = LIST_OR;
        } else if (strcmp(";", (*tokens)->text) == 0) {
            list_type = LIST_SEQ;
        }

        // If this is a subsequent call, advance pointer to the next (non-operand) token
        if (list_type != LIST_NONE) {
            (*tokens)++;
        }

        listAst->list.operators[listAst->list.count] = list_type;
        listAst->list.pipelines[listAst->list.count] = convert_pipeline(tokens);
        listAst->list.count++;
    } while ((*tokens)->type == TOKEN_OPERAND);

    return listAst;
}
