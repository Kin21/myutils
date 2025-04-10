#include "internal_utils.h"
#include "dynamic_array.h"
#include "string.h"
#include "assert.h"

#ifndef ARGPARSE_HEADER
#define ARGPARSE_HEADER
#define MAX_ARG_KEY_SIZE 24
#define MAX_ARG_VALUE_SIZE 128
#define NOT_FLAG 0
#define IS_FLAG 1
#define FLAG_SET 2
#define ARG_OPTIONAL 4
#define VALUE_SET 8
#define DEFAULT_VALUE VALUE_SET
#define KEY_NOT_FOUND -1

struct
{
    char key[MAX_ARG_KEY_SIZE];
    unsigned char flag;
    char value[MAX_ARG_VALUE_SIZE];
    char *help_msg;
    size_t id;
} typedef Argument, *pArgument;

struct
{
    size_t count;
    size_t capacity;
    pArgument array;
    char *footer_msg;
    char *epilog;
} typedef Arglist, *pArglist;

// Flag shows if value should be present, or if argument is optional/flag
void push_argument(pArglist arg_list, Argument arg);
// Print switch block for arguments
void print_arguments_switch_skeleton(pArglist arg_list);
int parse_arguments(int argc, char **argv, pArglist arg_list);
unsigned char is_flag_set(pArglist arg_list, char *key);
unsigned char is_value_set(pArglist arg_list, char *key);
// Prints help for parsed arguments
void print_default_help(pArglist arg_list);

// Return index of found argument by it's key or KEY_NOT_FOUND if not found
static int index_argument_by_key(pArglist arg_list, char key[]);
// Checks if all mandatory arguments are set
static int check_if_all_args_set(pArglist arg_list);
// Hash on key
static size_t create_id_from_key(char *key);

#ifdef ARGPARSE_HEADER_IMPLEMENTATION

void print_default_help(pArglist arg_list)
{
    if (arg_list->footer_msg)
        printf("%s\n", arg_list->footer_msg);

    for (size_t i = 0; i < arg_list->count; ++i)
    {
        printf("\t%s\t%-*s\n", arg_list->array[i].key,
            MAX_ARG_KEY_SIZE, arg_list->array[i].help_msg ? arg_list->array[i].help_msg : arg_list->array[i].key);
        if (arg_list->array[i].flag & DEFAULT_VALUE)
            printf("%*s %s\n", MAX_ARG_KEY_SIZE, "DEFAULT:", arg_list->array[i].value);
    }

    if (arg_list->epilog)
        printf("\n%s\n", arg_list->epilog);
}

unsigned char is_flag_set(pArglist arg_list, char *key)
{
    int i;
    i = index_argument_by_key(arg_list, key);
    return i == KEY_NOT_FOUND ? 0 : (arg_list->array[i].flag & FLAG_SET);
}

unsigned char is_value_set(pArglist arg_list, char *key)
{
    int i;
    i = index_argument_by_key(arg_list, key);
    return i == KEY_NOT_FOUND ? 0 : (arg_list->array[i].flag & VALUE_SET);
}

static int check_if_all_args_set(pArglist arg_list)
{
    for (size_t i = 0; i < arg_list->count; ++i)
    {
        if (!(arg_list->array[i].flag & ARG_OPTIONAL) && !(arg_list->array[i].flag & VALUE_SET))
        {
            report_error_and_exit("mandatory argument not set '%s'", arg_list->array[i].key);
        }
    }
    return 0;
}

int parse_arguments(int argc, char **argv, pArglist arg_list)
{
    int i, arg_i;

    for (i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            arg_i = index_argument_by_key(arg_list, argv[i]);
            if (arg_i == KEY_NOT_FOUND)
                goto argument_not_found;
        }
        if (arg_list->array[arg_i].flag & IS_FLAG)
            arg_list->array[arg_i].flag |= FLAG_SET;
        else
        {
            if (i + 1 >= argc || index_argument_by_key(arg_list, argv[i + 1]) != KEY_NOT_FOUND)
                goto value_not_provided;
            i++;
            if (strlen(argv[i]) >= MAX_ARG_VALUE_SIZE)
                goto value_length_to_big;
            strcpy(arg_list->array[arg_i].value, argv[i]);
            arg_list->array[arg_i].flag |= VALUE_SET;
        }
    }
    return check_if_all_args_set(arg_list);

// Error handling block
argument_not_found:
    report_error_and_exit("provided argument is not defined '%s'\n", argv[i]);
value_length_to_big:
    report_error_and_exit("size of the input argument value is too big.\n");
value_not_provided:
    report_error_and_exit("value for argument %s is not provided.\n", argv[i]);

    assert(1 != 1);
}

static size_t create_id_from_key(char *key)
{
    char c;
    size_t hash = 0;
    while ((c = *key++) != '\0')
        hash = c + (hash << 6) + (hash << 16) - hash;
    return hash;
}
void push_argument(pArglist arg_list, Argument arg)
{
    if (arg.flag & IS_FLAG || arg.flag & DEFAULT_VALUE)
        arg.flag |= ARG_OPTIONAL;
    arg.id = create_id_from_key(arg.key);
    append(Argument, *arg_list, arg);
}

static int index_argument_by_key(pArglist arg_list, char key[])
{
    for (size_t i = 0; i < arg_list->count; ++i)
        if (!strcmp(key, arg_list->array[i].key))
            return i;
    return KEY_NOT_FOUND;
}

void print_arguments_switch_skeleton(pArglist arg_list)
{
    printf("switch (arg)\n\t{\n");
    for (size_t i = 0; i < arg_list->count; ++i)
    {
        printf("\t// %s\n\tcase %zu:\n\tbreak;\n", arg_list->array[i].key, arg_list->array[i].id);
    }
    printf("default:\n\tbreak;\n\t}\n");
}

#endif // ARGPARSE_HEADER_IMPLEMENTATION

#endif // ARGPARSE_HEADER