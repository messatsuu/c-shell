#ifndef AST_PARSE_STATE_H
#define AST_PARSE_STATE_H

#include "ast/ast.h"
#include "tokenizer/token.h"

typedef enum {
    TYPE_AST,
    TYPE_TOKEN,
} ParseType;

typedef struct ParseState {
    // The member which contains the parsing-logic
    union {
        Token *tokens;
        AST *listAst;
    } parsable;

    ParseType type;
    char **errors;
    unsigned int error_count;
} ParseState;

bool print_errors(ParseState *parseState);
void report_error(ParseState *parseState, char *message);
void init_parse_state(ParseState *parseState, ParseType parseType);
void cleanup_parse_state(ParseState *parseState);

#endif

