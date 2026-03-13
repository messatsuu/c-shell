#include "ast/ast.h"
#include "ast/ast_executor.h"
#include "command/alias.h"
#include "core/prompt.h"
#include "core/settings.h"
#include "core/shell.h"
#include "cshread/history.h"
#include "cshread/input.h"
#include "parser/ast_parser.h"
#include "parser/parse_state.h"
#include "parser/parser.h"
#include "tokenizer/tokenizer.h"
#include "utility.h"

#include <cshread/cshread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

extern int last_exit_code;
extern Settings *settings;

char *get_user_input() {
    char *original_input = cshr_read_input(get_prompt());
    if (!original_input) {
        return nullptr;
    }

    // handle multiline-input with backslash
    while (original_input[strlen(original_input) - 1] == '\\') {
        // remove backslash
        original_input[strlen(original_input) - 1] = '\0';

        // get more input with PS2
        char *new_input = cshr_read_input(getenv("PS2"));
        size_t original_input_length = strlen(original_input);

        // append to input
        ensure_capacity((void **)&original_input, &original_input_length, original_input_length, strlen(new_input), sizeof(char));
        strncat(original_input, new_input, strlen(new_input));

        free(new_input);
    }

    return original_input;
}

// Main Loop of the shell.
// Expects `original_input` to be a dynamically allocated string
void execute_input(char *original_input) {
    // GNU readline and c-shell-read both return NULL on EOF (CTRL+D), should this be handled differently?
    if (original_input == NULL) {
        exit(EXIT_SUCCESS);
    }

    if (strlen(original_input) == 0) {
        free(original_input);
        return;
    }

    char *mutated_input = strdup(original_input);
    mutate_original_input(&mutated_input);
    expand_aliases(&mutated_input);

    // Early initialize variables for usage in cleanup
    ParseState *tokenParseState = nullptr;
    ParseState *astParseState = nullptr;
    const Token *baseTokenPointer = nullptr;

    // mutating input failed, cleanup
    if (!mutated_input) {
        goto cleanup;
    }

    tokenParseState = tokenize(mutated_input);
    baseTokenPointer = tokenParseState->parsable.tokens;

    // empty input or errors, cleanup
    if (baseTokenPointer->type == TOKEN_EOF || print_errors(tokenParseState)) {
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

    if (settings->track_history) {
        cshr_history_append(original_input);
    }

    free(original_input);
    free(mutated_input);
}

// flush terminal input and communicate sigint-capture to cshr
void reset_shell() {
    printf("\n");
    fflush(stdout);
    cshr_sigint_received = 1;
}
