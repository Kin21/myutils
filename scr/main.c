#include <stdio.h>
#include "dynamic_array.h"

#define INTERNAL_UTILS_IMPLEMENTATION
#include "internal_utils.h"

#define ARGPARSE_HEADER_IMPLEMENTATION
#include "argparse.h"

int main(int argc, char **argv)
{
    Arglist arg_list = {0};

    push_argument(&arg_list, (Argument){.key = "-k", .flag = IS_FLAG, .help_msg="Test k flag."});
    push_argument(&arg_list, (Argument){.key = "-f", .value = "/etc/passwd", .flag = DEFAULT_VALUE});
    push_argument(&arg_list, (Argument){.key = "-h", .flag = IS_FLAG});

    parse_arguments(argc, argv, &arg_list);

    if (is_flag_set(&arg_list, "-h"))
        print_default_help(&arg_list);
    printf("%s", arg_list.array[index_argument_by_key(&arg_list,"-f")].value);
    free_array(arg_list);
    return 0;
}