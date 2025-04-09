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
void report_error_and_exit(char error_msg[]);

// #define INTERNAL_UTILS_IMPLEMENTAION
#ifdef INTERNAL_UTILS_IMPLEMENTAION

int read_line_to_buff(unsigned char *buff, size_t buff_max_size, size_t *bytes_read, FILE *f)
{
    char c;
    size_t i;

    i = 0;
    while ((c = getc(f)) != '\n' && c != EOF && i < buff_max_size)
        buff[i++] = c;
    if (i == buff_max_size)
    {
        printf("Degug - %zu\n", i);
        report_error_and_exit("provided buffer size is too small to handle input line.");
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
    buff->array[i] = '\0';
    *bytes_read = i;
    return c;
}

void report_error_and_exit(char error_msg[])
{
    fprintf(stderr, "CRITICAL ERROR: %s\n", error_msg);
    exit(11);
}

#endif // INTERNAL_UTILS_IMPLEMENTAION

#endif // INTERNAL_UTILS