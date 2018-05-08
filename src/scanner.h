#ifndef _scanner_h
#define _scanner_h

#include "trie.h"

typedef enum {
    TOK_EOF,
    TOK_OP,
    TOK_INT,
    TOK_FLOAT,
    TOK_ID,
    TOK_FALSE,
    TOK_TRUE,

    TOK_LESS,
    TOK_LESSEQUAL,
    TOK_EQUAL,
    TOK_NOTEQUAL,
    TOK_GREATER,
    TOK_GREATEREQUAL

} token_type_t;

typedef struct {
    token_type_t type;
    int line;
    union {
        int i_value;
        float f_value;
        char* lexeme;
    };
} token_t;

typedef struct {
    int line;
    trie_node_t* keywords;
    char* source;
    char* peek;
} scanner_t;

void scanner_init(scanner_t* scanner, char* source);
void scanner_free(scanner_t* scanner);

void scanner_scan(scanner_t* scanner, token_t* tok);

#endif
