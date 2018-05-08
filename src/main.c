#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "trie.h"

typedef enum {
    TOK_EOF,
    TOK_OP,
    TOK_NUM,
    TOK_ID,
    TOK_FALSE,
    TOK_TRUE
} token_type_t;

typedef struct {
    token_type_t type;
    int line;
    union {
        int value;
        char* lexeme;
    };
} token_t;

char* read_file(const char* filename)
{
    FILE* f = fopen(filename, "r");
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buffer = malloc(length + 1);
    if (buffer)
    {
        memset(buffer, '\0', length + 1);
        fread(buffer, 1, length, f);
    }
    fclose(f);
    return buffer;
}

#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define IS_LETTER(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) >= 'Z'))

int line = 1;
char* peek = NULL;
trie_node_t* words = NULL;

void scan(token_t* tok)
{
#define MATCH(chr) (*peek == (chr))
#define NEXT(chr) (*(peek + 1) == (chr))
    for (; *peek; ++peek)
    {
        if (MATCH(' ') || MATCH('\t'))
            continue;
        else if (MATCH('\n'))
            line++;
        else if (MATCH('/') && NEXT('/'))
        {
            while (!MATCH('\n'))
                ++peek;
            --peek;
        }
        else if (MATCH('/') && NEXT('*'))
        {
            ++peek;
            ++peek;
            while (!(MATCH('*') && NEXT('/')))
            {
                if (MATCH('\n'))
                    line++;
                ++peek;
                if (!*peek)
                {
                    tok->line = line;
                    tok->type = TOK_EOF;
                    return;
                }
            }
            ++peek;
        }
        else
            break;
    }
#undef MATCH
#undef NEXT

    tok->line = line;

    if (!*peek)
    {
        tok->type = TOK_EOF;
        return;
    }

    if (IS_DIGIT(*peek))
    {
        int v = 0;
        do {
            v = 10 * v + (*peek - '0');
            ++peek;
        } while (IS_DIGIT(*peek));

        tok->type = TOK_NUM;
        tok->value = v;
        return;
    }
    else if (IS_LETTER(*peek))
    {
        char* start = peek;
        do {
            ++peek;
        } while (IS_LETTER(*peek) || IS_DIGIT(*peek));

        size_t len = (peek - start);
        char* word = malloc(len + 1);
        memset(word, '\0', len + 1);
        strncpy(word, start, len);

        int type = trie_contains(words, word);

        if (type >= 0)
        {
            free(word);
            word = NULL;
            tok->type = type;
            return;
        }

        trie_insert(&words, word, TOK_ID);

        tok->type = TOK_ID;
        tok->lexeme = word;
        return;
    }

    tok->type = TOK_OP;
    tok->value = *peek++;
}

int main(int argc, const char* argv[])
{
    if (argc != 2)
    {
        printf("expected file path\n");
    }

    char* content = read_file(argv[1]);
    if (!content)
    {
        printf("empty file\n");
        return 1;
    }
    peek = content;
    trie_insert(&words, "true", TOK_TRUE);
    trie_insert(&words, "false", TOK_FALSE);

    token_t tok;
    do {
        scan(&tok);
        printf("<%i: ", tok.line);
        switch (tok.type)
        {
        case TOK_ID:
            printf("ID, %s>\n", tok.lexeme);
            free(tok.lexeme);
            tok.lexeme = NULL;
            break;
        case TOK_NUM:
            printf("NUM, %i>\n", tok.value);
            break;
        case TOK_EOF:
            printf("EOF>\n");
            break;
        case TOK_OP:
            printf("OP, %c>\n", tok.value);
            break;
        case TOK_FALSE:
            printf("false>\n");
            break;
        case TOK_TRUE:
            printf("true>\n");
            break;
        default:
            printf("%c>\n", tok.value);
            break;
        }
    } while (tok.type != TOK_EOF);

    free(content);
    content = NULL;
    trie_free(&words);

    return 0;
}
