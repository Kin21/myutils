#include "stdio.h"
#include "stdlib.h"
#include "dynamic_array.h"

#ifndef INTERNAL_UTILS
#define INTERNAL_UTILS

struct
{
    size_t count;
    size_t capacity;
    unsigned char *array;
} typedef uCharArray, *puCharArray;

// Reads line to the buffer, returns '\n' or EOF when finished.
// Drops \n in read line, and null terminates it.
// If size of the buffer is too small to handle the line, exits using 'report_error_and_exit' function.
int read_line_to_buff(unsigned char *buff, size_t buff_max_size, size_t *bytes_read, FILE *f);

// Reads line to the buffer, returns '\n' or EOF when finished.
// Drops \n in read line, and null terminates it.
// Expects buffer to be dynamic array
int read_line(puCharArray buff, size_t *bytes_read, FILE *f);

// Prints error message to stderr, finish program with 11 status code
#define report_error_and_exit(format, error_msg...)   \
    fprintf(stderr, "%s", "Error: ");                 \
    fprintf(stderr, format __VA_OPT__(, ) error_msg); \
    exit(11)

// #define INTERNAL_UTILS_IMPLEMENTATION
#ifdef INTERNAL_UTILS_IMPLEMENTATION

int read_line_to_buff(unsigned char *buff, size_t buff_max_size, size_t *bytes_read, FILE *f)
{
    char c;
    size_t i;

    i = 0;
    while ((c = getc(f)) != '\n' && c != EOF && i < buff_max_size)
        buff[i++] = c;
    if (i == buff_max_size)
    {
        report_error_and_exit("%s", "provided buffer size is too small to handle input line.");
    }
    buff[i] = '\0';
    *bytes_read = i;
    return c;
}

int read_line(puCharArray buff, size_t *bytes_read, FILE *f)
{
    char c;
    size_t i;

    i = 0;
    buff->count = 0;
    while ((c = getc(f)) != '\n' && c != EOF)
    {
        append(unsigned char, *buff, c);
        ++i;
    }
    append(unsigned char, *buff, '\0');
    append(unsigned char, *buff, '\0');
    *bytes_read = i;
    return c;
}

#endif // INTERNAL_UTILS_IMPLEMENTATION

#endif // INTERNAL_UTILS