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
    PAIR,
    SYMBOL
} object_type;

typedef struct object object;

struct object {
    object_type type;
    int tag;
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
        char* symbol;
    } data;
};

typedef struct object_list object_list;

struct object_list {
    object* obj;
    object_list* next;
};

object_list* objects;
int current_tag = 0;
unsigned int allocations = 0;
unsigned int sweep_limit = 4;

object* empty_list;
object* obj_true;
object* obj_false;
object* symbol_table;
object* quote_symbol;
object* define_symbol;
object* set_symbol;
object* ok_symbol;
object* empty_environment;
object* global_environment;

void add_to_object_list(object* obj)
{
    object_list* entry = malloc(sizeof(object_list));
    if (!entry)
    {
        fprintf(stderr, "out of memory\n");
        _exit(1);
    }
    entry->obj = obj;
    entry->next = objects;
    objects = entry;
    allocations++;
}

object* make_object()
{
    object* obj = malloc(sizeof(object));
    if (!obj)
    {
        fprintf(stderr, "out of memory\n");
        _exit(1);
    }
    obj->tag = 0;
    // add to objects list
    add_to_object_list(obj);
    
    return obj;
}

void free_object(object* obj)
{
    if (!obj)
        return;
    allocations--;
    switch (obj->type)
    {
    case PAIR:
        //free_object(obj->data.pair.car);
        //free_object(obj->data.pair.cdr);
        break;
    case STRING:
        free(obj->data.string);
        break;
    case SYMBOL:
        free(obj->data.symbol);
        break;
    }
    free(obj);
}

char is_type(object* obj, object_type type)
{
    return obj->type == type;
}

char is_false(object* obj)
{
    return obj == obj_false;
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
    obj->data.string = calloc(strlen(value) + 1, sizeof(char));
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

void set_car(object* obj, object* val)
{
    obj->data.pair.car = val;
}

void set_cdr(object* obj, object* val)
{
    obj->data.pair.cdr = val;
}

object* make_symbol(char* value)
{
    object* element = symbol_table;
    // is symbol already in table?
    while (element != empty_list)
    {
        object* first = car(element);
        if (strcmp(first->data.symbol, value) == 0)
            return first;
        element = cdr(element);
    }

    // make new object and add it to symbol table
    object* obj = make_object();
    obj->type = SYMBOL;
    obj->data.symbol = calloc(strlen(value) + 1, sizeof(char));
    if (!obj->data.symbol)
    {
        fprintf(stderr, "out of memory\n");
        _exit(1);
    }
    strcpy(obj->data.symbol, value);
    symbol_table = cons(obj, symbol_table);
    return obj;
}

object* first_frame(object* env)
{
    return car(env);
}

object* frame_variables(object* frame)
{
    return car(frame);
}

object* frame_values(object* frame)
{
    return cdr(frame);
}

object* make_frame(object* vars, object* vals)
{
    return cons(vars, vals);
}

void add_binding_to_frame(object* var, object* val, object* frame)
{
    set_car(frame, cons(var, car(frame)));
    set_cdr(frame, cons(val, cdr(frame)));
}

object* extend_environment(object* vars, object* vals, object* base_env)
{
    return cons(make_frame(vars, vals), base_env);
}

object* lookup_variable_value(object* var, object* env)
{
    while (env != empty_environment)
    {
        object* frame = first_frame(env);
        object* vars = frame_variables(frame);
        object* vals = frame_values(frame);

        while (vars != empty_list)
        {
            if (var == car(vars))
            {
                return car(vals);
            }
            vars = cdr(vars);
            vals = cdr(vals);
        }
        // enclosing environment
        env = cdr(env);
    }
    fprintf(stderr, "unbound variable\n");
    _exit(1);
}

void set_variable_value(object* var, object* val, object* env)
{
    while (env != empty_environment)
    {
        object* frame = first_frame(env);
        object* vars = frame_variables(frame);
        object* vals = frame_values(frame);

        while (vars != empty_list)
        {
            if (var == car(vars))
            {
                set_car(vals, val);
                return;
            }
            vars = cdr(vars);
            vals = cdr(vals);
        }
        // enclosing environment
        env = cdr(env);
    }
    fprintf(stderr, "unbound variable\n");
    _exit(1);
}

void define_variable(object* var, object* val, object* env)
{
    object* frame = first_frame(env);
    object* vars = frame_variables(frame);
    object* vals = frame_values(frame);

    while (vars != empty_list)
    {
        if (var == car(vars))
        {
            // already defined?
            set_car(vals, val);
            return;
        }
        vars = cdr(vars);
        vals = cdr(vals);
    }
    add_binding_to_frame(var, val, frame);
}

void init()
{
    objects = NULL;

    empty_list = make_object();
    empty_list->type = EMPTY_LIST;

    obj_true = make_object();
    obj_true->type = BOOLEAN;
    obj_true->data.boolean = 1;

    obj_false = make_object();
    obj_false->type = BOOLEAN;
    obj_false->data.boolean = 0;

    symbol_table = empty_list;
    quote_symbol = make_symbol("quote");
    define_symbol = make_symbol("define");
    set_symbol = make_symbol("set!");
    ok_symbol = make_symbol("ok");

    empty_environment = empty_list;
    global_environment = extend_environment(empty_list, empty_list, empty_environment);
}

void mark_object(object* obj)
{
    if (!obj)
        return;
    obj->tag = current_tag;
    if (is_type(obj, PAIR))
    {
        mark_object(car(obj));
        mark_object(cdr(obj));
    }
}

void mark()
{
    current_tag++;

    mark_object(empty_list);
    mark_object(obj_true);
    mark_object(obj_false);
    mark_object(symbol_table);
    mark_object(empty_environment);
    mark_object(global_environment);
}

void sweep()
{
    // is object at head of objects list untagged?
    while (objects && objects->obj->tag != current_tag)
    {
        object_list* head = objects;
        objects = objects->next;
        free_object(head->obj);
        free(head);
    }

    if (!objects)
        return;

    object_list* prev = objects;
    object_list* current = objects->next;
    while (current)
    {
        if (current->obj->tag != current_tag)
        {
            free_object(current->obj);

            object_list* entry = current;
            prev->next = current->next;
            current = current->next;
            free(entry);
            continue;
        }
        prev = current;
        current = current->next;
    }
}

/* READ */

char is_delimiter(int c)
{
    return isspace(c) || c == EOF ||
        c == '(' || c == ')' ||
        c == '"' || c == ';';
}

char is_initial(int c)
{
    return isalpha(c) || c == '*' || c == '/' || c == '>' ||
        c == '<' || c == '=' || c == '?' || c == '!';
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
        return empty_list;
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
 
#define BUFFER_MAX 1024
    char buffer[BUFFER_MAX];

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
    else if (is_initial(c) ||
        ((c == '-' || c == '+') && is_delimiter(peek(in))))
    {
        int len = 0;
        while (is_initial(c) || isdigit(c) ||
            c == '+' || c == '-')
        {
            /* subtract 1 to save space for '\0' terminator */
            if (len < BUFFER_MAX - 1)
                buffer[len++] = c;
            else
            {
                fprintf(stderr, "symbol too long. Maximum length is %d\n", BUFFER_MAX);
                _exit(1);
            }
            c = getc(in);
        }
        if (is_delimiter(c))
        {
            buffer[len] = '\0';
            ungetc(c, in);
            return make_symbol(buffer);
        }
        fprintf(stderr, "symbol not followed by delimiter. Found '%c'\n", c);
        _exit(1);
    }
    else if (c == '"')
    {
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
                fprintf(stderr, "string too long. max length: %d\n", BUFFER_MAX);
                _exit(1);
            }
        }
        buffer[len] = '\0';
        return make_string(buffer);
    }
    else if (c == '(')
    {
        return read_pair(in);
    }
    else if (c == '\'')
    {
        return cons(quote_symbol, cons(read(in), empty_list));
    }
    else
    {
        fprintf(stderr, "bad input. Unexpected '%c'\n", c);
        _exit(1);
    }
    fprintf(stderr, "read illegal state\n");
    _exit(1);
#undef BUFFER_MAX
}

/* EVAL */

#define cadr(obj) car(cdr(obj))
#define caddr(obj) car(cdr(cdr(obj)))

char is_self_evaluating(object* exp)
{
    return is_type(exp, BOOLEAN) ||
        is_type(exp, INTEGER) ||
        is_type(exp, FLOATING) ||
        is_type(exp, CHARACTER) ||
        is_type(exp, STRING);
}

char is_tagged_list(object* expression, object* tag)
{
    if (is_type(expression, PAIR))
    {
        object* the_car = car(expression);
        return is_type(the_car, SYMBOL) && (the_car == tag);
    }
    return 0;
}

char is_variable(object* exp)
{
    return is_type(exp, SYMBOL);
}

char is_quoted(object* exp)
{
    return is_tagged_list(exp, quote_symbol);
}

char is_assignment(object* exp)
{
    return is_tagged_list(exp, set_symbol);
}

char is_definition(object* exp)
{
    return is_tagged_list(exp, define_symbol);
}

object* text_of_quotation(object* exp)
{
    return cadr(exp);
}

object* eval(object* exp, object* env);

object* eval_assignment(object* exp, object* env)
{
    set_variable_value(cadr(exp), eval(caddr(exp), env), env);
    return ok_symbol;
}

object* eval_definition(object* exp, object* env)
{
    define_variable(cadr(exp), eval(caddr(exp), env), env);
    return ok_symbol;
}

object* eval(object* exp, object* env)
{
    if (is_self_evaluating(exp))
        return exp;
    else if (is_variable(exp))
        return lookup_variable_value(exp, env);
    else if (is_quoted(exp))
        return text_of_quotation(exp);
    else if (is_assignment(exp))
        return eval_assignment(exp, env);
    else if (is_definition(exp))
        return eval_definition(exp, env);
    else
    {
        fprintf(stderr, "cannot eval unknown expression type\n");
        _exit(1);
    }
    fprintf(stderr, "eval illegal state\n");
    _exit(1);
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
    case SYMBOL:
        printf("%s", obj->data.symbol);
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

void do_gc()
{
    //printf("allocations: %u\n", allocations);
    if (allocations >= sweep_limit)
    {
        mark();
        sweep();

        sweep_limit = allocations + allocations / 2;

        //printf("allocations after collection: %u\nnew sweep limit: %u\n", allocations, sweep_limit);
    }
}

int main(int argc, char* argv[])
{
    init();

    while (1)
    {
        printf("> ");
        write(eval(read(stdin), global_environment));
        printf("\n");

        do_gc();
    }
}

void panic_exit(const char* message, char* file, int line)
{
    current_tag++;
    sweep();

    printf("allocations after final sweep: %u\n", allocations);

    fprintf(stderr, "exiting in file %s at line %i\n", file, line);

    exit(1);
}
