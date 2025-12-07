#include "ast/ast.h"
#include "parser/ast_parser.h"
#include "parser/parse_state.h"
#include "tokenizer/tokenizer.h"
#include "utility.h"
#include <stdio.h>
#include <string.h>

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

AST *convert_simple_command(ParseState *parseState, const Token **tokens) {
    AST *simpleCommandAst = allocate(sizeof(AST), true);
    simpleCommandAst->type = NODE_SIMPLE;

    unsigned int allocated_elements_count = INITIAL_BUFSIZE;
    simpleCommandAst->simple.argv = (char **)callocate(allocated_elements_count, sizeof(char *), true);
    simpleCommandAst->simple.redirection = nullptr;

    unsigned int argc = 0;

    while ((*tokens)->type == TOKEN_WORD || (*tokens)->type == TOKEN_REDIRECT || (*tokens)->type == TOKEN_REDIRECT) {
        // reallocate argv and double space if needed
        if (argc >= allocated_elements_count - 1) {
            simpleCommandAst->simple.argv = (char **)reallocate_safe(simpleCommandAst->simple.argv, allocated_elements_count * sizeof(char *), 2 * allocated_elements_count * sizeof(char *), true);
            allocated_elements_count *= 2;
        }

        if ((*tokens)->type == TOKEN_SUBSHELL_START) {
            report_error(parseState, (char *)"Syntax Error: unexpected (");
            goto return_simple_command;
        }

        if ((*tokens)->type == TOKEN_REDIRECT) {
            // Next token needs to be the redirect-file
            if ((*tokens + 1)->type != TOKEN_WORD) {
                report_error(parseState, (char *)"Syntax Error: expected token");
                goto return_simple_command;
            }

            Redirection *redirection = allocate(sizeof(Redirection), true);
            redirection->redirect_filename = allocate(strlen((*tokens + 1)->text) + 1, true);
            strcpy(redirection->redirect_filename, (*tokens + 1)->text);

            redirection->type = get_redir_type((*tokens)->text);
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

AST *convert_pipeline(ParseState *parseState, const Token **tokens) {
    AST *pipelineAst = allocate(sizeof(AST), true);
    pipelineAst->type = NODE_PIPELINE;
    pipelineAst->pipeline.command_count = 0;
    pipelineAst->pipeline.commands = (AST **)allocate(INITIAL_BUFSIZE * sizeof(AST *), true);

    while ((*tokens)->type == TOKEN_WORD) {
        AST *simpleCommandAst = convert_simple_command(parseState, tokens);
        if (simpleCommandAst == nullptr) {
            return pipelineAst;
        }
        pipelineAst->pipeline.commands[pipelineAst->pipeline.command_count++] = simpleCommandAst;

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

    for (unsigned int i = 0; i < listAst->list.pipeline_count; i++) {
        printf("List %d:\n", i);

        printf("pipeline:\n");
        AST *pipeline = listAst->list.pipelines[i];

        printf("\tcount: %d\n", pipeline->pipeline.command_count);
        printf("\toperator: %d\n", listAst->list.operators[i]);

        for (unsigned int j = 0; j < pipeline->pipeline.command_count; j++) {
            printf("\tsimpleCommand %d:\n", j);
            AST *simpleCommand = pipeline->pipeline.commands[j];

            unsigned int k = 0;
            while (simpleCommand->simple.argv[k] != nullptr) {
                printf("\t\targ[%d]: %s\n", k, simpleCommand->simple.argv[k]);
                k++;
            }

            if (simpleCommand->simple.redirection) {
                printf("\t\tredirection-type: %d\n", simpleCommand->simple.redirection->type);
                printf("\t\tredirection-filename: %s\n", simpleCommand->simple.redirection->redirect_filename);
            }
        }
    }
}

// Main entry-point to parsing the tokenized input
ParseState *convert_tokens_to_ast(const Token **tokens) {
    ParseState *parseState = allocate(sizeof(ParseState), true);
    init_parse_state(parseState, TYPE_AST);

    AST *listAst = allocate(sizeof(AST), true);
    listAst->type = NODE_LIST;
    listAst->list.operators = allocate(INITIAL_BUFSIZE * sizeof(ListType *), true);
    listAst->list.pipelines = (AST **)allocate(INITIAL_BUFSIZE * sizeof(AST *), true);
    listAst->list.pipeline_count = 0;

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

        listAst->list.is_subshell = (*tokens)->type == TOKEN_SUBSHELL_START;
        listAst->list.operators[listAst->list.pipeline_count] = list_type;
        listAst->list.pipelines[listAst->list.pipeline_count] = convert_pipeline(parseState, tokens);
        listAst->list.pipeline_count++;
    } while ((*tokens)->type == TOKEN_OPERAND);

    parseState->parsable.listAst = listAst;
    return parseState;
}
