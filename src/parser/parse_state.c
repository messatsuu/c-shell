#include "parser/parse_state.h"
#include "ast/ast.h"
#include "string.h"
#include "tokenizer/tokenizer.h"
#include "utility.h"
#include <stdlib.h>

bool print_errors(ParseState *parseState) {
    for (unsigned int i = 0; i < parseState->error_count; i++) {
        fprintf(stderr, "%s\n", parseState->errors[i]);
    }

    return parseState->error_count > 0;
}

void report_error(ParseState *parseState, char *message) {
    parseState->errors[parseState->error_count++] = strdup(message);
}

void init_parse_state(ParseState *parseState, ParseType parseType) {
    parseState->errors = (char **)callocate(INITIAL_BUFSIZE, sizeof(char *), true);
    parseState->error_count = 0;
    parseState->type = parseType;
    // TODO: does it matter which member of the union we assign nullptr here?
    parseState->parsable.listAst = nullptr;
}

void cleanup_parse_state(ParseState *parseState) {
    if (parseState == nullptr) {
        return;
    }

    switch (parseState->type) {
        case TYPE_AST:
            cleanup_node_type(parseState->parsable.listAst);
            break;
        case TYPE_TOKEN:
            cleanup_tokens(parseState->parsable.tokens);
            break;
    }

    for (unsigned int i = 0; i < parseState->error_count; i++) {
        free(parseState->errors[i]);
    }
    free(parseState->errors);
    free(parseState);

    parseState = nullptr;
}
