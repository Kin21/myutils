#include "internal_utils.h"
#include "argparse.h"
#include "ctype.h"

#ifndef TEE_HEADER
#define TEE_HEADER

int tee_main(int argc, char **argv);
static int tee_implementation(pArglist arg_list);

#define TEE_HEADER_IMPLEMENTATION
#ifdef TEE_HEADER_IMPLEMENTATION

int tee_main(int argc, char **argv)
{
    Arglist arg_list = {.footer_msg = "tee - read from standard input and write to standard output and files"
                                      "tee [OPTION(s)] [FILE(s)]\nCopy standard input to each FILE, and also to standard output.",
                        .epilog = "By default files that cannot be opened will be ignored, but if cannot write to one of files execution stops."};
    push_argument(&arg_list, (Argument){.key = "-h", .flag = IS_FLAG, .help_msg = "Prints this help message."});
    push_argument(&arg_list, (Argument){.key = "-a", .flag = IS_FLAG, .help_msg = "Opens FILE(s) in append mode."});
    parse_arguments(argc, argv, &arg_list);
    if (is_flag_set(&arg_list, "-h"))
    {
        print_default_help(&arg_list);
        exit(0);
    }
    else
        tee_implementation(&arg_list);

    return 0;
}

static int tee_implementation(pArglist arg_list)
{
    uCharArray line = {0};
    struct Files
    {
        size_t capacity;
        size_t count;
        FILE **array;
    };
    struct Files files = {0};
    size_t bytes_read, bytes_wrote, next_file;
    FILE *hFile;
    char *f;
    bytes_read = bytes_wrote = next_file = 0;

    while ((f = get_next_positional_value(arg_list, &next_file)) != NULL)
    {
        hFile = fopen(f, is_flag_set(arg_list, "-a") ? "a" : "w");
        if (!hFile)
        {
            fprintf(stderr, "Warning: cannot open file %s\n", f);
            continue;
        }
        append(FILE*, files, hFile);
    }
    append(FILE*, files, stdout);

    while (read_line(&line, &bytes_read, stdin) != EOF)
    {
        for (size_t i = 0; i < files.count; ++i)
        {
            bytes_wrote = fwrite(line.array, sizeof(unsigned char), line.count, files.array[i]);
            if (bytes_wrote != (bytes_read + 2)) // +2 as read_line drops \n and do not count \0 
            {
                report_error_and_exit("W:%zu R:%zu cannot write to specified file\n", bytes_wrote, bytes_read);
            }
            putc('\n', files.array[i]);
        }
    }

    while ((hFile = pop(files, NULL)) != NULL)
        fclose(hFile);
    free_array(files);
    free_array(line);

    return 0;
}

#endif // TEE_HEADER_IMPLEMENTATION

#endif // TEE_HEADER