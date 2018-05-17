#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "scanner.h"

#include "hashtable.h"

#include <time.h>

#include <vld.h>

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

static void put_test(hashtable_t* ht, size_t num)
{
    long start = clock();

    char name[31];
    name[30] = '\0';
    for (size_t i = 0; i < num; i++)
    {
        for (int j = 0; j < 30; j++)
        {
            name[j] = (char)(48 + rand() % 78);
        }
        ht_put(ht, name, NULL, 0);
    }

    long end = clock();

    double dur = end - start;
    printf("putting %zu took %f s.\n", num, dur / CLOCKS_PER_SEC);
}

int main(int argc, const char* argv[])
{
#if 1
    {
        hashentry_t t;
        printf("%i\n", sizeof(t));
#ifdef HASHTABLE_INCLUDE_KEY_IN_ENTRY
        printf(" %i\n", sizeof(t.key_value));
#endif
        printf(" %i\n", sizeof(t.key));
        printf(" %i\n", sizeof(t.free_data));
        printf(" %i\n", sizeof(t._pad));
        printf(" %i\n", sizeof(t.data));
        printf(" %i\n", sizeof(t.next));
    }

    hashtable_t* ht = ht_new(4096);

    put_test(ht, 10);
    ht_clear(ht);
    put_test(ht, 100);
    ht_clear(ht);
    put_test(ht, 1000);
    ht_clear(ht);
    put_test(ht, 10000);
    ht_clear(ht);
    put_test(ht, 100000);
    //ht_clear(ht);

    ht_repl(ht);

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
