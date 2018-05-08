#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "scanner.h"

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

int main(int argc, const char* argv[])
{
    if (argc != 2)
    {
        printf("expected file path\n");
        return 1;
    }

    char* content = read_file(argv[1]);
    if (!content)
    {
        printf("empty file\n");
        return 1;
    }

    scanner_t scanner;
    scanner_init(&scanner, content);

    token_t tok;
    do {
        scanner_scan(&scanner, &tok);
        printf("<%i: ", tok.line);
        switch (tok.type)
        {
        case TOK_ID:
            printf("ID, %s>\n", tok.lexeme);
            free(tok.lexeme);
            tok.lexeme = NULL;
            break;
        case TOK_INT:
            printf("INT, %i>\n", tok.i_value);
            break;
        case TOK_FLOAT:
            printf("FLOAT, %f>\n", tok.f_value);
            break;
        case TOK_EOF:
            printf("EOF>\n");
            break;
        case TOK_OP:
            printf("OP, %c>\n", tok.i_value);
            break;
        case TOK_FALSE:
            printf("false>\n");
            break;
        case TOK_TRUE:
            printf("true>\n");
            break;
        default:
            printf("%c>\n", tok.i_value);
            break;
        }
    } while (tok.type != TOK_EOF);

    scanner_free(&scanner);

    return 0;
}
