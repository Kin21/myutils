#include <stdio.h>
#include "dynamic_array.h"

#define INTERNAL_UTILS_IMPLEMENTATION
#include "internal_utils.h"

#define ARGPARSE_HEADER_IMPLEMENTATION
#include "argparse.h"

int main(int argc, char **argv)
{
    Arglist arg_list = {0};

    push_argument(&arg_list, (Argument){.key = "-k", .flag = IS_FLAG});
    push_argument(&arg_list, (Argument){.key = "-f", .value = "/etc/passwd", .flag = DEFAULT_VALUE});
    push_argument(&arg_list, (Argument){.key = "-h", .flag = IS_FLAG});

    parse_arguments(argc, argv, &arg_list);
    for (size_t i = 0; i < arg_list.count; ++i)
    {
        printf("Key: %s, Value: %s, FLAG_SET: %u, VALUE_SET: %u\n", arg_list.array[i].key, arg_list.array[i].value,
               is_flag_set(&arg_list, arg_list.array[i].key),
               is_value_set(&arg_list, arg_list.array[i].key));
    }

    return 0;
}