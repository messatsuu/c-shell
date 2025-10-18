#ifndef TOKENIZER_H
#define TOKENIZER_H

typedef enum {
    TOKEN_WORD,
    TOKEN_OPERAND,
    TOKEN_PIPE,
    TOKEN_REDIRECT,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char *text;  // dynamically allocated string holding the token
} Token;

Token *tokenize(const char *input);

void cleanup_tokens(Token *tokens);

#endif
