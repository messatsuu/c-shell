#ifndef AST_PARSER_H
#define AST_PARSER_H

#include "ast/ast.h"
#include "tokenizer/tokenizer.h"

typedef struct ASTParseState {
    AST *listAst;
    char **errors;
    unsigned int error_count;
} ASTParseState;

void cleanup_ast_parse_state();
ASTParseState *convert_tokens_to_ast(const Token **tokens);

#endif
