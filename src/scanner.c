#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <string.h>

#include "scanner.h"

void scanner_init(scanner_t* scanner, char* source)
{
    scanner->line = 1;
    scanner->words = NULL;
    scanner->source = source;
    scanner->peek = source;

    trie_insert(&(scanner->words), "true", TOK_TRUE);
    trie_insert(&(scanner->words), "false", TOK_FALSE);
}

void scanner_free(scanner_t* scanner)
{
    scanner->line = 0;
    scanner->peek = NULL;
    trie_free(&(scanner->words));
    if (scanner->source)
    {
        free(scanner->source);
        scanner->source = NULL;
    }
}

#define IS_DIGIT(c) ((c) >= '0' && (c) <= '9')
#define IS_LETTER(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) >= 'Z'))

#define PEEK() (*(scanner->peek))
#define PEEK_NEXT() (*(scanner->peek + 1))

#define BACK() (--scanner->peek)
#define ADVANCE() (++scanner->peek)

#define MATCH(chr) (PEEK() == (chr))
#define MATCH_NEXT(chr) (PEEK_NEXT() == (chr))

static void match_number(scanner_t* scanner, token_t* tok)
{
    int v = 0;
    do {
        v = 10 * v + (PEEK() - '0');
        ADVANCE();
    } while (IS_DIGIT(PEEK()));

    if (MATCH('.'))
    {
        float f = (float)v;
        int order = 10;
        ADVANCE();
        while (IS_DIGIT(PEEK()))
        {
            f += ((PEEK() - '0') / (float)order);
            order *= 10;
            ADVANCE();
        }

        tok->type = TOK_FLOAT;
        tok->f_value = f;
        return;
    }

    tok->type = TOK_INT;
    tok->i_value = v;
}

static void match_word(scanner_t* scanner, token_t* tok)
{
    char* start = scanner->peek;
    do {
        ADVANCE();
    } while (IS_LETTER(PEEK()) || IS_DIGIT(PEEK()));

    size_t len = (scanner->peek - start);
    char* word = malloc(len + 1);
    memset(word, '\0', len + 1);
    strncpy(word, start, len);

    int type = trie_contains(scanner->words, word);

    if (type >= 0)
    {
        free(word);
        word = NULL;
        tok->type = type;
        return;
    }

    trie_insert(&(scanner->words), word, TOK_ID);

    tok->type = TOK_ID;
    tok->lexeme = word;
}

void scanner_scan(scanner_t* scanner, token_t* tok)
{
    for (; PEEK(); ADVANCE())
    {
        if (MATCH(' ') || MATCH('\t'))
            continue;
        else if (MATCH('\n'))
            scanner->line++;
        else if (MATCH('/') && MATCH_NEXT('/'))
        {
            while (!MATCH('\n'))
                ADVANCE();
            BACK();
        }
        else if (MATCH('/') && MATCH_NEXT('*'))
        {
            ADVANCE();
            ADVANCE();
            while (!(MATCH('*') && MATCH_NEXT('/')))
            {
                if (MATCH('\n'))
                    scanner->line++;
                ADVANCE();
                if (MATCH('\0'))
                {
                    tok->line = scanner->line;
                    tok->type = TOK_EOF;
                    return;
                }
            }
            ADVANCE();
        }
        else
            break;
    }

    tok->line = scanner->line;

    if (MATCH('\0'))
    {
        tok->type = TOK_EOF;
        return;
    }

    if (IS_DIGIT(PEEK()))
    {
        match_number(scanner, tok);
        return;
    }
    else if (IS_LETTER(PEEK()))
    {
        match_word(scanner, tok);
        return;
    }

    tok->type = TOK_OP;
    tok->i_value = PEEK();
    ADVANCE();
}

#undef IS_DIGIT
#undef IS_LETTER
#undef PEEK
#undef PEEK_NEXT
#undef BACK
#undef ADVANCE
#undef MATCH
#undef MATCH_NEXT
