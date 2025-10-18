#include "ast/ast.h"
#include "parser/ast_parser.h"
#include "tokenizer/tokenizer.h"
#include "utility.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ASTParseState *parseState = nullptr;

void report_error(char *message) {
    parseState->errors[parseState->error_count++] = strdup(message);
}

RedirType get_redir_type(char *text) {
    RedirType type = REDIR_OUT;

    if (strcmp(text, ">>") == 0) {
        type = REDIR_OUT_APP;
    }

    if (strcmp(text, "<") == 0) {
        type = REDIR_IN;
    }

    return type;
}

AST *convert_simple_command(const Token **tokens) {
    AST *simpleCommandAst = allocate(sizeof(AST), true);
    simpleCommandAst->type = NODE_SIMPLE;

    simpleCommandAst->simple.argv = (char **)allocate(INITIAL_BUFSIZE * sizeof(char **), true);
    simpleCommandAst->simple.redirection = nullptr;

    unsigned int argc = 0;

    while ((*tokens)->type == TOKEN_WORD || (*tokens)->type == TOKEN_REDIRECT) {
        if ((*tokens)->type == TOKEN_REDIRECT) {
            // Next token needs to be the redirect-file
            if ((*tokens + 1)->type != TOKEN_WORD) {
                report_error((char *)"Syntax Error: expected token");
                goto return_simple_command;
            }

            Redirection *redirection = allocate(sizeof(Redirection), true);
            redirection->type = get_redir_type((*tokens)->text);
            char *file_mode = nullptr;

            // TODO: implement REDIR_IN
            switch (redirection->type) {
                case REDIR_OUT_APP:
                    file_mode = (char *)"a";
                    break;
                default:
                    file_mode = (char *)"w";
            }

            redirection->redirect_file = fopen((*tokens + 1)->text, file_mode);

            simpleCommandAst->simple.redirection = redirection;
            (*tokens) += 2;
            continue;
        }
        simpleCommandAst->simple.argv[argc++] = strdup((*tokens)->text);
        (*tokens)++;
    }

return_simple_command:
    simpleCommandAst->simple.argv[argc++] = nullptr;
    return simpleCommandAst;
}

AST *convert_pipeline(const Token **tokens) {
    AST *pipelineAst = allocate(sizeof(AST), true);
    pipelineAst->type = NODE_PIPELINE;
    pipelineAst->pipeline.count = 0;
    pipelineAst->pipeline.commands = (AST **)allocate(INITIAL_BUFSIZE * sizeof(AST *), true);

    while ((*tokens)->type == TOKEN_WORD) {
        AST *simpleCommandAst = convert_simple_command(tokens);
        if (simpleCommandAst == nullptr) {
            return pipelineAst;
        }
        pipelineAst->pipeline.commands[pipelineAst->pipeline.count++] = simpleCommandAst;

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

ASTParseState *convert_tokens_to_ast(const Token **tokens) {
    parseState = allocate(sizeof(ASTParseState), true);
    parseState->errors = callocate(INITIAL_BUFSIZE, sizeof(char *), true);
    parseState->error_count = 0;
    parseState->listAst = nullptr;

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

    parseState->listAst = listAst;
    return parseState;
}

void cleanup_ast_parse_state() {
    cleanup_ast_list(parseState->listAst);

    for (unsigned int i = 0; i < parseState->error_count; i++) {
        free(parseState->errors[i]);
    }
    free(parseState->errors);
    free(parseState);
}
