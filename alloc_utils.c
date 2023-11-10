#include "alloc_utils.h"

/*  FUNCTION: alloc_2d
    INPUT:  size_t x, number of rows
            size_t y, number of columns
    OUTPUT: 2-dimensional array of characters.

    A 2-d array of characters is allocated by malloc, if the allocation fails (i.e. the returned pointer is NULL), an error message is printed out and the program exits.

*/
char **alloc_2d(size_t x, size_t y)
{
    char **out_2d = malloc(x * sizeof(*out_2d));
    if (out_2d == NULL)
    {
        perror("Error allocating the matrix");
        exit(EXIT_FAILURE);
    }
    for (size_t i = 0; i < x; i++)
    {
        out_2d[i] = calloc(y, sizeof(**out_2d));
        if (out_2d[i] == NULL)
        {
            perror("Error allocating a row");
            exit(EXIT_FAILURE);
        }
    }

    return out_2d;
}

/*  FUNCTION free_2d
    INPUT: a pointer to a 2D array of type char
           x the number of rows of the array
    OUTPUT: void

    The function frees the memory allocated for the 2D array. It works with arrays created with malloc.

*/
void free_2d(char **arr_2d, size_t x)
{

    for (size_t i = 0; i < x; i++)
    {
        free(arr_2d[i]);
    }
    free(arr_2d);
}

/*  FUNCTION resize_buffer
    INPUT:  a pointer to a char pointer **buffer
            a size_t variable linelen, which represents a required length of the buffer
    OUTPUT: the resized buffer

    If the original buffer is NULL or its length is less than linelen, the function resizes the buffer using realloc. If the resizing is successful, the function returns the new buffer. If the resizing fails, it frees the original buffer, outputs an error message using perror, and exits the program with failure status.
*/
char *resize_buffer(char **buffer, size_t linelen)
{
    if (*buffer == NULL || strlen(*buffer) < linelen)
    {
        char *buffer2 = realloc(*buffer, linelen + 1);
        if (buffer2 != NULL)
            *buffer = buffer2;
        else
        {
            free(*buffer);
            perror("Error reallocating");
            exit(EXIT_FAILURE);
        }
    }
    return *buffer;
}