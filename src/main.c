#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <vld.h>

#include "config.h"
stack_allocator_t* global_stack_alloc;

#define _exit(c) panic_exit("", __FILE__, __LINE__)

typedef enum {
    INTEGER,
    FLOATING,
    BOOLEAN,
    CHARACTER,
    STRING,
    EMPTY_LIST,
    PAIR
} object_type;

typedef struct object object;

struct object {
    object_type type;
    union {
        char boolean;
        char character;
        long integer;
        float floating;
        char* string;
        struct {
            object* car;
            object* cdr;
        } pair;
    } data;
};

object* obj_empty_list;
object* obj_true;
object* obj_false;

object* make_object()
{
    object* obj = malloc(sizeof(object));
    if (!obj)
    {
        fprintf(stderr, "out of memory\n");
        _exit(1);
    }
    return obj;
}

void free_object(object* obj)
{
    if (!obj || obj == obj_empty_list || obj == obj_false || obj == obj_true)
        return;
    if (obj->type == PAIR)
    {
        free_object(obj->data.pair.car);
        free_object(obj->data.pair.cdr);
    }
    else if (obj->type == STRING)
    {
        free(obj->data.string);
    }
    free(obj);
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

object* make_character(char value)
{
    object* obj = make_object();
    obj->type = CHARACTER;
    obj->data.character = value;
    return obj;
}

object* make_string(char* value)
{
    object* obj = make_object();
    obj->type = STRING;
    obj->data.string = malloc(strlen(value) + 1);
    if (!obj->data.string)
    {
        fprintf(stderr, "out of memory\n");
        _exit(1);
    }
    strcpy(obj->data.string, value);
    return obj;
}

object* cons(object* car, object* cdr)
{
    object* obj = make_object();
    obj->type = PAIR;
    obj->data.pair.car = car;
    obj->data.pair.cdr = cdr;
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

object* car(object* obj)
{
    if (is_type(obj, PAIR))
    {
        return obj->data.pair.car;
    }
    fprintf(stderr, "object not a pair\n");
    _exit(1);
}

object* cdr(object* obj)
{
    if (is_type(obj, PAIR))
    {
        return obj->data.pair.cdr;
    }
    fprintf(stderr, "object not a pair\n");
    _exit(1);
}

void init()
{
    obj_empty_list = make_object();
    obj_empty_list->type = EMPTY_LIST;

    obj_true = make_object();
    obj_true->type = BOOLEAN;
    obj_true->data.boolean = 1;

    obj_false = make_object();
    obj_false->type = BOOLEAN;
    obj_false->data.boolean = 0;
}

/* READ */

char is_delimiter(int c)
{
    return isspace(c) || c == EOF ||
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
        if (isspace(c))
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

void peek_character_end(FILE *in)
{
    int c = getc(in);
    if (c != '\'')
    {
        fprintf(stderr, "expected ' after character literal\n");
        _exit(1);
    }
    if (!is_delimiter(peek(in)))
    {
        fprintf(stderr, "character not followed by delimiter\n");
        _exit(1);
    }
}

object* read_character(FILE* in)
{
    int c = getc(in);

    switch (c)
    {
    case EOF:
        fprintf(stderr, "incomplete character literal\n");
        _exit(1);
    case '\\':
        c = getc(in);
        if (c == 'n')
        {
            peek_character_end(in);
            return make_character('\n');
        }
        fprintf(stderr, "unknown escaped character literal\n");
        _exit(1);
    }
    char val = c;
    peek_character_end(in);
    return make_character(val);
}

object* read(FILE* in);

object* read_pair(FILE* in)
{
    eat_whitespace(in);

    int c = getc(in);
    if (c == ')')
    {
        return obj_empty_list;
    }
    ungetc(c, in);

    object* car_obj = read(in);

    eat_whitespace(in);

    c = getc(in);
    if (c == '.')
    {
        /* read improper list */
        c = peek(in);
        if (!is_delimiter(c))
        {
            fprintf(stderr, "dot not followed by delimiter\n");
            _exit(1);
        }
        object* cdr_obj = read(in);
        eat_whitespace(in);
        c = getc(in);
        if (c != ')')
        {
            fprintf(stderr, "where was the trailing right paren?\n");
            _exit(1);
        }
        return cons(car_obj, cdr_obj);
    }
    /* read list */
    ungetc(c, in);
    object* cdr_obj = read_pair(in);
    return cons(car_obj, cdr_obj);
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
        case '\'':
            return read_character(in);
        default:
            fprintf(stderr, "Unknown boolean type\n");
            _exit(1);
        }
    }
    else if (isdigit(c) || (c == '-' && isdigit(peek(in))))
    {
        // integer
        short sign = 1;
        if (c == '-')
            sign = -1;
        else
            ungetc(c, in);

        long num = 0;
        while (isdigit(c = getc(in)))
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
            while (isdigit(c = getc(in)))
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
                _exit(1);
            }
        }
        else
        {
            fprintf(stderr, "number not followed by delimiter\n");
            _exit(1);
        }
    }
    else if (c == '"')
    {
#define BUFFER_MAX 1024
        char buffer[BUFFER_MAX];
        int len = 0;

        while ((c = getc(in)) != '"')
        {
            if (c == '\\')
            {
                c = getc(in);
                if (c == 'n')
                    c = '\n';
            }
            if (c == EOF)
            {
                fprintf(stderr, "unterminated string literal\n");
                _exit(1);
            }
            if (len < BUFFER_MAX - 1)
                buffer[len++] = c;
            else
            {
                fprintf(stderr, "string too long. max length: %i\n", BUFFER_MAX);
                _exit(1);
            }
        }
        buffer[len] = '\0';
        return make_string(buffer);

#undef BUFFER_MAX
    }
    else if (c == '(')
    {
        return read_pair(in);
    }
    else
    {
        fprintf(stderr, "bad input. Unexpected '%c'\n", c);
        _exit(1);
    }
    fprintf(stderr, "read illegal state\n");
    _exit(1);
}

/* EVAL */

object* eval(object* exp)
{
    return exp;
}

/* PRINT */

void write(object* obj);

void write_pair(object* pair)
{
    object* car_obj = car(pair);
    object* cdr_obj = cdr(pair);
    write(car_obj);
    if (is_type(cdr_obj, PAIR))
    {
        printf(" ");
        write_pair(cdr_obj);
    }
    else if (is_type(cdr_obj, EMPTY_LIST))
        return;
    else
    {
        printf(" . ");
        write(cdr_obj);
    }
}

void write(object* obj)
{
    switch (obj->type)
    {
    case BOOLEAN:
        printf("#%c", is_false(obj) ? 'f' : 't');
        break;
    case CHARACTER:
        printf("#'%c'", obj->data.character);
        break;
    case EMPTY_LIST:
        printf("()");
        break;
    case INTEGER:
        printf("%ld", obj->data.integer);
        break;
    case FLOATING:
        printf("%f", obj->data.floating);
        break;
    case STRING: {
            char* str = obj->data.string;
            putchar('"');
            while (*str)
            {
                switch (*str)
                {
                case '\n':
                    printf("\\n");
                    break;
                case '\\':
                    printf("\\\\");
                    break;
                case '"':
                    printf("\\\"");
                    break;
                default:
                    putchar(*str);
                }
                str++;
            }
            putchar('"');
        }
        break;
    case PAIR:
        printf("(");
        write_pair(obj);
        printf(")");
        break;
    default:
        fprintf(stderr, "unknown object type\n");
        _exit(1);
    }
}

int main(int argc, char* argv[])
{
    init();

    while (1)
    {
        printf("> ");
        object* expr = eval(read(stdin));
        write(expr);
        printf("\n");
        free_object(expr);
    }
}

void panic_exit(const char* message, char* file, int line)
{
    free(obj_empty_list);
    free(obj_true);
    free(obj_false);

    fprintf(stderr, "exiting in file %s at line %i\n", file, line);

    exit(1);
}
