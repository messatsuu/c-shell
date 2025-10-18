#include "tokenizer/tokenizer.h"
#include "utility.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int is_operand_character(char character) {
    return (character == '|' || character == '&' || character == ';' || character == '<' || character == '>');
}

static TokenType get_operand_token_type(const char *input) {
    TokenType type = TOKEN_OPERAND;

    switch (*input) {
        case '|':
            type = TOKEN_PIPE;
            break;
        // TODO: handle this better
        case '>':
            type = TOKEN_REDIRECT;
            break;
    }

    return type;
}

static unsigned int get_operand_length(const char *input) {
    // all currently supported multi-char operators
    const char *operators[] = {
        "||", "&&", ">>",
        nullptr
    };

    for (int i = 0; operators[i]; i++) {
        unsigned int operator_length = strlen(operators[i]);
        if (strncmp(input, operators[i], operator_length) == 0) {
            return operator_length;
        }
    }

    // Default: single-character operator
    return 1;
}

void debug_print_tokens(Token *tokens) {
    unsigned int i = 0;
    printf("TOKENIZATION:\n");
    while (tokens[i].type != TOKEN_EOF) {
        printf("TYPE: %u, TEXT: %s, LENGTH: %lu\n", tokens[i].type, tokens[i].text, strlen(tokens[i].text));
        i++;
    }
    printf("\n\n");
}

Token *tokenize(const char *input) {
    unsigned int allocated_elements_count = INITIAL_BUFSIZE;
    unsigned int count = 0;
    Token *tokens = allocate(allocated_elements_count * sizeof(Token), true);

    for (unsigned int i = 0; input[i]; ) {
        const char current_char = input[i];
        // Skip whitespace
        if (isspace(current_char)) {
            i++;
            continue;
        }

        // Reallocate token-array if needed
        if (count + 1 >= allocated_elements_count) {
            tokens = reallocate_safe(tokens, allocated_elements_count * sizeof(Token), (allocated_elements_count * 2) * sizeof(Token), true);
            allocated_elements_count *= 2;
        }

        // Operators
        if (is_operand_character(current_char)) {
            unsigned int operand_length = get_operand_length(input + i);
            char *text = strndup(input + i, operand_length);
            TokenType type = get_operand_token_type(input + i);

            tokens[count++] = (Token){type, text};
            i += operand_length;
            continue;
        }

        // Words (with quote handling)
        unsigned int j = i;
        char buffer[INITIAL_BUFSIZE_BIG];
        unsigned int k = 0;

        while (input[j] && !isspace((unsigned char)input[j]) && !is_operand_character(input[j])) {
            if (input[j] != '\'' && input[j] != '"') {
                buffer[k++] = input[j++];
                continue;
            }

            // Quote handling; put all arguments in token and retain quotes
            char quote_character = input[j];
            buffer[k++] = input[j++]; // Copy the opening quote

            // Copy until we hit the matching closing quote or end of input
            while (input[j] && input[j] != quote_character) {
                buffer[k++] = input[j++];
            }

            if (input[j] == quote_character) {
                buffer[k++] = input[j++];
            }
        }
        buffer[k] = '\0';
        tokens[count++] = (Token){TOKEN_WORD, strdup(buffer)};
        i = j;
    }

    if (count + 1 >= allocated_elements_count) {
        tokens = reallocate_safe(tokens, allocated_elements_count * sizeof(Token), (allocated_elements_count * 2) * sizeof(Token), true);
        allocated_elements_count *= 2;
    }

    // EOF token
    tokens[count++] = (Token){TOKEN_EOF, nullptr};

    return tokens;
}

void cleanup_tokens(Token *tokens) {
    unsigned int i = 0;
    while (tokens[i].type != TOKEN_EOF) {
        free(tokens[i++].text);
    }

    free(tokens);
}
