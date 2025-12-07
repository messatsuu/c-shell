#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "parser/parse_state.h"

ParseState *tokenize(const char *input);

void cleanup_tokens(const Token *tokens);

#endif
