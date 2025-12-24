#include "tokenizer/tokenizer.h"
#include "parser/parse_state.h"
#include "tokenizer/token.h"
#include "utility.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool is_operand_character(char character) {
    return (character == '|' || character == '&' || character == ';' || character == '<' || character == '>');
}

static bool is_subshell_character(char character) {
    return (character == '(' || character == ')');
}

static bool is_quote_character(char character) {
    return (character == '\'' || character == '\"');
}

static bool is_escape_character(char character) {
    return character == '\\';
}

// Checks if a character is considered a "special character" (characters that can be escaped)
static bool is_special_character(char character) {
    return is_quote_character(character) ||
        is_operand_character(character) ||
        is_subshell_character(character) ||
        is_escape_character(character)
    ;
}

static TokenType get_operand_token_type(const char *input, unsigned int operand_length) {
    TokenType type = TOKEN_OPERAND;

    switch (*input) {
        case '>':
        case '<':
            type = TOKEN_REDIRECT;
            break;
    }

    if (operand_length == 1) {
        switch (*input) {
            case '|':
                type = TOKEN_PIPE;
                break;
        }
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

ParseState *tokenize(const char *input) {
    unsigned int allocated_elements_count = INITIAL_BUFSIZE;
    unsigned int count = 0;

    ParseState *parseState = callocate(sizeof(ParseState), 1, true);
    init_parse_state(parseState, TYPE_TOKEN);

    Token *tokens = callocate(allocated_elements_count, sizeof(Token), true);

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
            TokenType type = get_operand_token_type(input + i, operand_length);

            tokens[count++] = (Token){type, text};
            i += operand_length;
            continue;
        }

        // Subshells
        if (is_subshell_character(current_char)) {
            TokenType type = TOKEN_SUBSHELL_START;
            if (current_char == ')') {
                type = TOKEN_SUBSHELL_END;
            }
            char *token = malloc(2 * sizeof(char));
            token[0] = current_char;
            token[1] = '\0';
            tokens[count++] = (Token){type, token};

            i++;
            continue;
        }

        // Words (with quote handling)
        char buffer[INITIAL_BUFSIZE_BIG] = {};
        unsigned int k = 0;

        while (input[i] && !isspace((unsigned char)input[i]) && !is_operand_character(input[i]) && !is_subshell_character(input[i])) {
            if (!is_quote_character(input[i])) {
                // escaped characters add one more
                if (input[i] == '\\' && is_special_character(input[i + 1])) {
                    buffer[k++] = input[i++];
                }
                buffer[k++] = input[i++];
                continue;
            }

            // Quote handling; put all arguments in token and retain quotes
            char quote_character = input[i];
            buffer[k++] = input[i++]; // Copy the opening quote

            // Copy until we hit the matching closing quote or end of input
            while (input[i] && input[i] != quote_character) {
                buffer[k++] = input[i++];
            }

            if (input[i] == quote_character) {
                buffer[k++] = input[i++];
            } else {
                // TODO: actually allow for PS2 and closing quotes
                // Also, should the Tokenizer be the one to report that error?
                report_error(parseState, (char *)"Syntax Error: Unclosed Quote");
                goto return_parse_state;
            }
        }
        buffer[k] = '\0';
        tokens[count++] = (Token){TOKEN_WORD, strdup(buffer)};
    }

    if (count + 1 >= allocated_elements_count) {
        tokens = reallocate_safe(tokens, allocated_elements_count * sizeof(Token), (allocated_elements_count * 2) * sizeof(Token), true);
        allocated_elements_count *= 2;
    }

return_parse_state:
    // EOF token
    tokens[count++] = (Token){TOKEN_EOF, nullptr};
    parseState->parsable.tokens = tokens;

    return parseState;
}

void cleanup_tokens(const Token *tokens) {
    unsigned int i = 0;
    while (tokens[i].type != TOKEN_EOF) {
        free(tokens[i++].text);
    }

    // casting to drop const (freeing consts is illegal)
    free((Token *)tokens);
}
