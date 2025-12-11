#ifndef AST_H
#define AST_H

#include <stdio.h>
typedef enum {
    LIST_NONE, // just a single pipeline, list has only left side
    LIST_SEQ, // ;
    LIST_AND, // &&
    LIST_OR // ||
} ListType;

typedef enum {
    REDIR_OUT,     // >
    REDIR_OUT_APP, // >>
    REDIR_IN       // <
} RedirType;

typedef struct Redirection {
    RedirType type;
    char *redirect_filename;
} Redirection;

typedef enum {
    NODE_SIMPLE,
    NODE_SUBSHELL,
    NODE_PIPELINE,
    NODE_LIST
} NodeType;

// TODO: look into tagged structs and type-aliases
typedef struct AST {
    NodeType type;

    union {
        struct {
            char **argv; // null-terminated string-array
           struct Redirection *redirection;
        } simple;

        struct {
            int command_count;
            struct AST **commands; // Either type simple or subshell
        } pipeline;

        struct {
            int pipeline_count;
            ListType *operators;
            struct AST **pipelines;
        } list;

        struct {
            struct AST *list;
        } subshell;
    };
} AST;

void cleanup_node_type(AST *astNode);

#endif
