#ifndef AST_H
#define AST_H

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

// TODO: should this have `File` as a member?
typedef struct {
    RedirType type;
    char *filename;
} Redirection;

typedef enum {
    NODE_SIMPLE,
    NODE_PIPELINE,
    NODE_LIST
} NodeType;

// TODO: look into tagged structs and type-aliases
typedef struct AST {
    NodeType type;

    union {
        struct {
            char **argv; // e.g. {"ls", "-la", NULL}
            struct Redirection *redirection;
        } simple;

        struct {
            int count; // Number of pipes
            struct AST **commands;
        } pipeline;

        struct {
            int count; // Number of pipelines
            struct AST **pipelines;
            ListType *operators;
        } list;
    };
} AST;

#endif
