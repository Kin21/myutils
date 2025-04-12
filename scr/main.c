#include "string.h"
#define INTERNAL_UTILS_IMPLEMENTATION
#define ARGPARSE_HEADER_IMPLEMENTATION
#define WC_HEADER_IMPLEMENTATION
#include "wc.h"
#define TEE_HEADER_IMPLEMENTATION
#include "tee.h"
// Check if str2 is at the end of str1, return 0 if true
int backward_substr(char *str1, char *str2)
{
    char *s1_start, *s2_start;
    s1_start = str1;
    s2_start = str2;
    while (*str1 != '\0')
        str1++;
    while (*str2 != '\0')
        str2++;
    while (str1 != s1_start && str2 != s2_start)
    {
        if (*str1 != *str2)
            return *str1 > *str2 ? 1 : -1;
        str1--;
        str2--;
    }
    if (str2 == s2_start)
        return 0;
    return 1;
}

int main(int argc, char **argv)
{
    char *supported_programs[] = {"wc",
                                  "tee"};
    if (!backward_substr(argv[0], "wc"))
        wc_main(argc, argv);
    else if (!backward_substr(argv[0], "tee"))
        tee_main(argc, argv);
    else
    {
        printf("Program name '%s' not recognized\n", argv[0]);
        printf("Create link to main with name of suported program:\n");
        for (size_t i = 0; i < sizeof(supported_programs) / sizeof(char *); ++i)
            printf("\t%s\n", supported_programs[i]);
    }
    return 0;
}