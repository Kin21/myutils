#include "stdlib.h"
#include "stdio.h"

#ifndef DYNAMIC_ARRAY_HEADER
#define DYNAMIC_ARRAY_HEADER

// Number of elements added to array capacity if realloc needed
#define ARRAY_SCALING_SIZE 256 

#define free_array(a) free((a).array)

#define append(type, a, el)                                                                                  \
    if ((a).count == (a).capacity)                                                                           \
    {                                                                                                        \
        (a).array = (type *)realloc((a).array, sizeof(el) * ARRAY_SCALING_SIZE + sizeof(el) * (a).capacity); \
        (a).capacity += ARRAY_SCALING_SIZE;                                                                  \
    }                                                                                                        \
    (a).array[(a).count++] = el


// Pops an element from the array 'a', or returns 'if_zero' if empty
#define pop(a, if_zero) ((a).count > 0) ? ((a).array[--(a).count]) : (if_zero)

#define print_array(el_formatter, a)              \
    {                                             \
        for (int i = 0; i < (a).count; ++i)       \
        {                                         \
            printf((el_formatter), (a).array[i]); \
        }                                         \
    }

#endif // DYNAMIC_ARRAY_HEADER
