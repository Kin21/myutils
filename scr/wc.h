#include "internal_utils.h"
#include "argparse.h"
#include "ctype.h"

#ifndef WC_HEADER
#define WC_HEADER

// Entry for wc program
int wc_main(int argc, char **argv);
static void wc_on_file(char *f, size_t *total_lines, size_t *total_words, size_t *total_bytes, pArglist arg_list);
static void wc_implementation(pArglist arg_list);

#define WC_HEADER_IMPLEMENTATION
#ifdef WC_HEADER_IMPLEMENTATION

int wc_main(int argc, char **argv)
{
    Arglist arg_list = {.footer_msg = "Print newline, word, and byte counts for each FILE, and a total line if "
                                      "more than one FILE is specified.\nA word is a nonempty sequence of non white"
                                      "space delimited by white space characters or by start or end of input.\n"
                                      "Usage: wc [OPTION(s)] [FILE]\nWith no FILE, or when FILE is -, read standard input.",
                        .epilog = "Default delimeter is \\t"};

    push_argument(&arg_list, (Argument){.key = "-h", .flag = IS_FLAG, .help_msg = "Prints this help message."});
    push_argument(&arg_list, (Argument){.key = "-l", .flag = IS_FLAG, .help_msg = "Include lines number to output."});
    push_argument(&arg_list, (Argument){.key = "-w", .flag = IS_FLAG, .help_msg = "Include words number to output."});
    push_argument(&arg_list, (Argument){.key = "-b", .flag = IS_FLAG, .help_msg = "Include bytes number to output."});
    push_argument(&arg_list, (Argument){.key = "-d", .flag = DEFAULT_VALUE, .help_msg = "Delimiter for output.", .value = "\t"});
    push_argument(&arg_list, (Argument){.key = "-", .flag = IS_FLAG, .help_msg = "Use to read from stdin on some point."});
    parse_arguments(argc, argv, &arg_list);

    if (is_flag_set(&arg_list, "-h"))
    {
        print_default_help(&arg_list);
        exit(0);
    }
    else
    {
        wc_implementation(&arg_list);
    }

    free_array(arg_list);
    return 0;
}

static void wc_on_file(char *f, size_t *total_lines, size_t *total_words, size_t *total_bytes, pArglist arg_list)
{
    size_t file_lines, file_words, file_bytes;
    size_t line_words, line_bytes;
    unsigned char *tmp, in_word;

    file_lines = file_words = file_bytes = 0;
    file_lines = file_words = file_bytes = 0;
    line_words = line_bytes = 0;

    uCharArray line = {0};
    FILE *hFile = (strcmp(f, "-") == 0) ? stdin : fopen(f, "r");
    if (!hFile)
    {
        fprintf(stderr, "Error: cannot open and skipping file '%s'", f);
        return;
    }
    while (read_line(&line, &line_bytes, hFile) != EOF)
    {
        file_lines++;
        file_bytes += line_bytes + 1; // +1 as read_line drops \n

        tmp = line.array;
        in_word = 0;
        while (*tmp != '\0' && *(tmp + 1) != '\0')
        {
            if (isspace(*tmp))
            {
                in_word = 0;
                while (isspace(*tmp))
                    tmp++;
            }
            if (*tmp != '\0' && in_word == 0)
            {
                in_word = 1;
                line_words++;
            }
            tmp++;
        }
        file_words += line_words;
        line_words = line_bytes = 0;
    }
    *total_lines += file_lines;
    *total_bytes += file_bytes;
    *total_words += file_words;

    if (!is_flag_set(arg_list, "-l") && !is_flag_set(arg_list, "-w") && !is_flag_set(arg_list, "-b"))
        printf("%-zu%s%-zu%s%-zu%s%s\n", file_lines, get_value_by_key(arg_list, "-d"),
               file_words, get_value_by_key(arg_list, "-d"), file_bytes, get_value_by_key(arg_list, "-d"), f);
    else
    {
        if (is_flag_set(arg_list, "-l"))
            printf("%-zu%s", file_lines, get_value_by_key(arg_list, "-d"));
        if (is_flag_set(arg_list, "-w"))
            printf("%-zu%s", file_words, get_value_by_key(arg_list, "-d"));
        if (is_flag_set(arg_list, "-b"))
            printf("%-zu%s", file_bytes, get_value_by_key(arg_list, "-d"));
        printf("%s\n", f);
    }

    file_lines = file_words = file_bytes = 0;
    if (strcmp(f, "-"))
        fclose(hFile);
}

static void wc_implementation(pArglist arg_list)
{
    size_t total_lines, total_words, total_bytes;
    size_t current_file, files_read;
    char *f;

    total_lines = total_words = total_bytes = 0;
    files_read = current_file = 0;

    while ((f = get_next_positional_value(arg_list, &current_file)) != NULL)
    {
        wc_on_file(f, &total_lines, &total_words, &total_bytes, arg_list);
        files_read++;
    }
    if (!files_read)
    {
        wc_on_file("-", &total_lines, &total_words, &total_bytes, arg_list);
    }
    else if (files_read > 1)
    {
        if (!is_flag_set(arg_list, "-l") && !is_flag_set(arg_list, "-w") && !is_flag_set(arg_list, "-b"))
            printf("%-zu%s%-zu%s%-zu%s%s\n", total_lines, get_value_by_key(arg_list, "-d"),
                   total_words, get_value_by_key(arg_list, "-d"), total_bytes, get_value_by_key(arg_list, "-d"), "total");
        else
        {
            if (is_flag_set(arg_list, "-l"))
                printf("%-zu%s", total_lines, get_value_by_key(arg_list, "-d"));
            if (is_flag_set(arg_list, "-w"))
                printf("%-zu%s", total_words, get_value_by_key(arg_list, "-d"));
            if (is_flag_set(arg_list, "-b"))
                printf("%-zu%s", total_bytes, get_value_by_key(arg_list, "-d"));
            printf("%s\n", "total");
        }
    }
}

#endif // WC_HEADER_IMPLEMENTATION

#endif // WC_HEADER
