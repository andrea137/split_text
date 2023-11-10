#ifndef IO_UTILS_H
#define IO_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*  FUNCTION: read_one_line
    INPUT:  fin, a pointer to an input stream.
            out_line, poitner to the string where to write the processed lines.
            col_width, the width of a column.
    OUTPUT: the number of words in a line (newline considered as a single word) or EOF if fin reached the end.

    Function that reads and process one line from the stream fin and returns a string of words separated by a single space. Empty lines are converted in lines containing only the \n character. Is up to the caller to free out_line and open and close the stream.
*/
ssize_t read_one_line(FILE *fin, char **out_line, const int col_width);

#endif