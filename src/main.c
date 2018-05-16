#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "scanner.h"
#include "hashtable.h"

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
#if 1

    hashtable_t* ht = ht_new(16);

    ht_put(ht, "david", "david");
    ht_put(ht, "bavid", "bavid");
    ht_put(ht, "david2", "david2");
    ht_put(ht, "david3", "david3");

    char* str = NULL;
    if (ht_get(ht, "david", &str))
    {
        printf("got david: %s\n", str);
    }
    else
    {
        printf("david not in ht\n");
    }

    int* i = malloc(sizeof(int));
    *i = 15;

    ht_put(ht, "int", i);

    int* ii = NULL;
    if (ht_get(ht, "int", &ii))
    {
        printf("int: %i\n", *ii);
    }
    else
    {
        printf("int not in ht\n");
    }

    ht_remove(ht, "int");

    if (ht_get(ht, "int", &ii))
    {
        printf("int: %i\n", *ii);
    }
    else
    {
        printf("int not in ht\n");
    }

    free(i);

    char buffer[100];

    while (1)
    {
        printf("> ");
        char* input = fgets(buffer, 100, stdin);
        if (!input)
        {
            printf("error reading input!\n");
            break;
        }

        // remove trailing \n
        input[strlen(input) - 1] = '\0';

        if (strncmp(input, "exit", 5) == 0)
            break;

        if (strncmp(input, "get ", 4) == 0)
        {
            char* key = (input + 4);
            if (!*key)
            {
                printf("expected key!\n");
                continue;
            }

            char* value = NULL;
            if (ht_get(ht, key, &value))
                printf("'%s': %s\n", key, value);
            else
                printf("'%s' not in ht.\n", key);
        }

        if (strncmp(input, "put ", 4) == 0)
        {
            char* key = (input + 4);
            if (!*key)
            {
                printf("expected key!\n");
                continue;
            }
            char* value = key;
            while (*value && *value != ' ')
                value++;
            if (!*value)
            {
                printf("expected value!\n");
                continue;
            }
            *value = '\0';
            value++;

            ht_put(ht, key, _strdup(value));
        }
    }

    ht_free(&ht);
    return EXIT_SUCCESS;
#else
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
#endif
}


void panic_exit(const char* message, char* file, int line)
{
    printf("panic exit in file '%s' line %i:\n%s\n", file, line, message);
    sa_print_stats(global_stack_alloc);
    sa_free(&global_stack_alloc);
    exit(1);
}
