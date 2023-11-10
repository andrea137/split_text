#ifndef PROCESSING_H
#define PROCESSING_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/* FUNCTION: is_ascii
    INPUT: an unsigned character "c"
    OUTPUT: a boolean value.
    
    This function checks whether the input character is an ASCII character or not.
*/
bool is_ascii(unsigned char);

/*      The struct contains 3 fields:
        size_t i - represents the current row.
        size_t j - represents the current column.
        char *line_ptr - a pointer to the next character to be read in the current line.

        The purpose of this struct is to store the state of the processing procedure that is being performed on the text input by the process_one_line function. By storing the current row and column as well as the next character to be read, the program can keep track of where it is in the input and continue processing from that point.
*/
typedef struct Pr_data
{
    size_t i;
    size_t j; 
    char  *line_ptr;
} Pr_data;

/*  FUNCTION: strdisplen
    INPUT: character array str
    OUTPUT: integer value of data type size_t.

    This funcion calculates the length of a string in terms of ASCII characters (i.e., the number of characters shown).
 */
size_t strdisplen(const char *);

/*  FUNCTION: fill_with_char
    INPUT: str - A pointer to the string array that needs to be filled with characters
          c - The character to be filled in the string array
          n - The number of characters to be filled in the string array
    OUTPUT: str, the inputed string pointer
    
    This function fills the "str" string pointer with "c" character n times and adds a null terminator at the end of the "str" string pointer.
*/
char *fill_with_char(char *, char, size_t);

/*  FUNCTION: process_one_line
    INPUT: the number of columns
           the width of the columns
           the number of rows
           the spacing between the columns
           an array of output lines
           a Pr_data struct which stores the current position.
    OUTPUT: returns the pos_data struct after processing the current line. This struct is used to keep track of the
            current position when processing a line longer than a page.
    
    This function processes the input line in pos_data.line_ptr and formats the text according to the given column width. The formatted line is then appended to the current row of the out_lines array.
*/
Pr_data process_one_line(int, int, int, int, char **, Pr_data);

#endif