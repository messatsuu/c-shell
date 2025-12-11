#include "ast/ast.h"
#include "core/shell.h"
#include "ast/ast_executor.h"
#include "cshread/history.h"
#include "cshread/input.h"
#include "parser/ast_parser.h"
#include "parser/parse_state.h"
#include "parser/parser.h"
#include "tokenizer/tokenizer.h"

#include <cshread/cshread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

extern int last_exit_code;

void execute_input(char *original_input) {
    // GNU readline and c-shell-read both return NULL on EOF (CTRL+D), should this be handled differently?
    if (original_input == NULL) {
        exit(0);
    }

    // Early initialize variable for usage in cleanup
    char *mutated_input = strdup(original_input);
    mutate_original_input(&mutated_input);

    ParseState *tokenParseState = nullptr;
    ParseState *astParseState = nullptr;
    const Token *baseTokenPointer = nullptr;

    // mutating input failed, cleanup
    if (!mutated_input) {
        goto cleanup;
    }

    tokenParseState = tokenize(mutated_input);
    baseTokenPointer = tokenParseState->parsable.tokens;

    // empty input, cleanup
    if (baseTokenPointer->type == TOKEN_EOF) {
        goto cleanup;
    }

    // errors, cleanup
    if (print_errors(tokenParseState)) {
        goto cleanup;
    }

    astParseState = convert_tokens_to_ast(&baseTokenPointer);
    AST *listAst = astParseState->parsable.listAst;

    // errors, cleanup
    if (print_errors(astParseState)) {
        goto cleanup;
    }

    execute_node_type(listAst);

cleanup:
    cleanup_parse_state(astParseState);
    cleanup_parse_state(tokenParseState);
    cshr_history_append(original_input);

    free(original_input);
    free(mutated_input);
}

// flush terminal input and communicate sigint-capture to cshr
void reset_shell() {
    printf("\n");
    fflush(stdout);
    cshr_sigint_received = 1;
}
