#ifndef AST_PARSER_H
#define AST_PARSER_H

#include "ast/ast.h"
#include "tokenizer/tokenizer.h"

AST *convert_tokens_to_ast(Token **tokens);

#endif
