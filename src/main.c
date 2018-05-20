#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <vld.h>

#include "config.h"
stack_allocator_t* global_stack_alloc;

typedef enum {
    INTEGER,
    FLOATING,
    BOOLEAN
} object_type;

typedef struct {
    object_type type;
    union {
        char boolean;
        long integer;
        float floating;
    } data;
} object;

object* obj_true;
object* obj_false;

object* make_object()
{
    object* obj = malloc(sizeof(object));
    if (!obj)
    {
        fprintf(stderr, "out of memory\n");
        exit(1);
    }
    return obj;
}

object* make_integer(long value)
{
    object* obj = make_object();
    obj->type = INTEGER;
    obj->data.integer = value;
    return obj;
}

object* make_floating(float value)
{
    object* obj = make_object();
    obj->type = FLOATING;
    obj->data.floating = value;
    return obj;
}

char is_type(object* obj, object_type type)
{
    return obj->type == type;
}

char is_false(object* obj)
{
    return obj == obj_false;
}

void init()
{
    obj_true = make_object();
    obj_true->type = BOOLEAN;
    obj_true->data.boolean = 1;

    obj_false = make_object();
    obj_false->type = BOOLEAN;
    obj_false->data.boolean = 0;
}

/* READ */

char is_digit(int c)
{
    return c >= '0' && c <= '9';
}

char is_space(int c)
{
    return c == ' ' || c == '\t' || c == '\n';
}

char is_delimiter(int c)
{
    return is_space(c) || c == EOF ||
        c == '(' || c == ')' ||
        c == '"' || c == ';';
}

int peek(FILE* in)
{
    int c = getc(in);
    ungetc(c, in);
    return c;
}

void eat_whitespace(FILE* in)
{
    int c;

    while ((c = getc(in)) != EOF)
    {
        if (is_space(c))
            continue;
        if (c == ';')
        {
            while (((c = getc(in)) != EOF) && (c != '\n'));
            continue;
        }
        ungetc(c, in);
        break;
    }
}

object* read(FILE* in)
{
    eat_whitespace(in);

    int c = getc(in);
    
    if (c == '#')
    {
        c = getc(in);
        switch (c)
        {
        case 't':
            return obj_true;
        case 'f':
            return obj_false;
        default:
            fprintf(stderr, "Unknown boolean type\n");
            exit(1);
        }
    }
    else if (is_digit(c) || (c == '-' && is_digit(peek(in))))
    {
        // integer
        short sign = 1;
        if (c == '-')
            sign = -1;
        else
            ungetc(c, in);

        long num = 0;
        while (is_digit(c = getc(in)))
            num = num * 10 + (c - '0');
        num *= sign;
        if (is_delimiter(c))
        {
            ungetc(c, in);
            return make_integer(num);
        }
        else if (c == '.')
        {
            float fnum = (float)num;
            int order = 10;
            //c = getc(in);
            while (is_digit(c = getc(in)))
            {
                fnum += (c - '0') / (float)order;
                order *= 10;
            }
            if (is_delimiter(c))
            {
                ungetc(c, in);
                return make_floating(fnum);
            }
            else
            {
                fprintf(stderr, "number not followed by delimiter\n");
                exit(1);
            }
        }
        else
        {
            fprintf(stderr, "number not followed by delimiter\n");
            exit(1);
        }
    }
    else
    {
        fprintf(stderr, "bad input. Unexpected '%c'\n", c);
        exit(1);
    }
    fprintf(stderr, "read illegal state\n");
    exit(1);
}

/* EVAL */

object* eval(object* exp)
{
    return exp;
}

/* PRINT */

void write(object* obj)
{
    switch (obj->type)
    {
    case BOOLEAN:
        printf("#%c", is_false(obj) ? 'f' : 't');
        break;
    case INTEGER:
        printf("%ld", obj->data.integer);
        break;
    case FLOATING:
        printf("%f", obj->data.floating);
        break;
    default:
        fprintf(stderr, "unknown object type\n");
        exit(1);
    }
}

int main(int argc, char* argv[])
{
    init();

    while (1)
    {
        printf("> ");
        write(eval(read(stdin)));
        printf("\n");
    }
}

void panic_exit(const char* message, char* file, int line)
{

}
