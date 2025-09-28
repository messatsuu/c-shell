#include "ast/ast.h"
#include "command/command.h"
#include "core/prompt.h"
#include "core/shell.h"
#include "cshread/history.h"
#include "cshread/input.h"
#include "parser/ast_parser.h"
#include "parser/parser.h"
#include "tokenizer/tokenizer.h"

#include <cshread/cshread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>

extern int last_exit_code;

void execute_input() {
    char *original_input = cshr_read_input(get_prompt());

    // GNU readline and c-shell-read both return NULL on EOF (CTRL+D), should this be handled differently?
    if (original_input == NULL) {
        exit(0);
    }

    if (strlen(original_input) == 0) {
        free(original_input);
        return;
    }

    mutate_original_input(&original_input);

    Token *tokens = tokenize(original_input);
    Token *baseTokenPointer = tokens;
    AST *astList = convert_tokens_to_ast(&tokens);

    execute_ast_list(astList);

    cshr_history_append(original_input);

    // Cleanup
    cleanup_ast_list(astList);
    cleanup_tokens(baseTokenPointer);
    free(original_input);
}

// flush terminal input and communicate sigint-capture to cshr
void reset_shell() {
    printf("\n");
    fflush(stdout);
    cshr_sigint_received = 1;
}

void create_prompt() {
    execute_input();
}
