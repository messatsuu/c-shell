#ifndef AST_PARSER_H
#define AST_PARSER_H

#include "parser/parse_state.h"
#include "tokenizer/tokenizer.h"

void cleanup_ast_parse_state();
ParseState *convert_tokens_to_ast(const Token **tokens);

#endif
