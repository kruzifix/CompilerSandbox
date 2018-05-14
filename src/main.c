#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "scanner.h"

#include "vld.h"

stack_allocator_t* global_stack_alloc = NULL;

char* read_file(const char* filename)
{
    FILE* f = fopen(filename, "r");
    if (!f)
        return NULL;
    fseek(f, 0, SEEK_END);
    long length = ftell(f);
    fseek(f, 0, SEEK_SET);
    char* buffer = _MALLOC(length + 1);
    if (!buffer)
    {
        EXIT("unable to alloc file buffer");
    }
    //memset(buffer, '\0', length + 1);
    fread(buffer, 1, length, f);
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

    global_stack_alloc = sa_new(1024);

    char* content = read_file(argv[1]);
    if (!content)
    {
        printf("empty file\n");

        sa_free(&global_stack_alloc);
        return 1;
    }

    scanner_t* scanner = scanner_new(content);
    if (!scanner)
    {
        EXIT("unable to create scanner\n");
    }

    token_t tok;
    do {
        scanner_scan(scanner, &tok);
        printf("<%i: ", tok.line);
        switch (tok.type)
        {
        case TOK_ID:
            printf("ID, %s>\n", tok.lexeme);
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
        case TOK_LESS:
            printf("LESS>\n");
            break;
        case TOK_LESSEQUAL:
            printf("LESSEQUAL>\n");
            break;
        case TOK_EQUAL:
            printf("EQUAL>\n");
            break;
        case TOK_NOTEQUAL:
            printf("NOTEQUAL>\n");
            break;
        case TOK_GREATER:
            printf("GREATER>\n");
            break;
        case TOK_GREATEREQUAL:
            printf("GREATEREQUAL>\n");
            break;
        default:
            printf("%c>\n", tok.i_value);
            break;
        }
    } while (tok.type != TOK_EOF);

    //scanner_free(&scanner);

    sa_print_stats(global_stack_alloc);
    sa_free(&global_stack_alloc);

    return 0;
}

void panic_exit(const char* message, char* file, int line)
{
    printf("panic exit in file '%s' line %i:\n%s\n", file, line, message);
    sa_print_stats(global_stack_alloc);
    sa_free(&global_stack_alloc);
    exit(1);
}
