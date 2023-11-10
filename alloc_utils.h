#ifndef ALLOC_UTILS_H
#define ALLOC_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*  FUNCTION: alloc_2d
    INPUT:  size_t x, number of rows
            size_t y, number of columns
    OUTPUT: 2-dimensional array of characters.

    Allocates a 2-d array of characters.
*/
char **alloc_2d(size_t, size_t);

/*  FUNCTION free_2d
    INPUT:  a pointer to a 2D array of type char
            the number of rows of the array
    OUTPUT: void

    The function frees the memory allocated for the 2D array. It works with arrays created with malloc.
*/
void free_2d(char **, size_t);

/*  FUNCTION resize_buffer
    INPUT:  a pointer to a char pointer **buffer
            a size_t variable linelen, which represents a required length of the buffer
    OUTPUT: the resized buffer

    Resize a buffer string to a given size.
*/
char *resize_buffer(char **, size_t);

#endif