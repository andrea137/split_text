#include "processing.h"

/* FUNCTION: is_ascii
    INPUT: an unsigned character "c"
    RETURN: a boolean value.

    This function checks whether the input character is an ASCII character or not. To do this, it looks at the first byte of the character and checks if it starts with "10". If it does, this means that the byte has been added for UTF-8 encoding and should not be counted as an ASCII character. To check if the first byte starts with "10", the function performs a bitwise shift of the byte to the right by six positions and then uses the XOR bitwise operator to compare the result with the value 2. The purpose of casting the input unsigned character as "unsigned" is to avoid the risk of the shift operator performing a fill with ones, which could create issues in the evaluation of the final output.

    If the check is successful and the input character is an ASCII character, the function returns "true". Otherwise, it returns "false".
*/
bool is_ascii(unsigned char c)
{
    return 2 ^ (c >> (sizeof(c) * 8 - 2));
}

/* FUNCTION: strdisplen
   INPUT: character array str
   OUTPUT: integer value of data type size_t.
   
   This funcion calculates the length of a string in terms of ASCII characters (i.e., the number of characters shown).
 */
size_t strdisplen(const char *str)
{
    int i = 0, cnt = 0;
    while (str[i] != '\0')
    {
        if (is_ascii(str[i]))
            cnt++;
        i++;
    }
    return cnt;
}

/* FUNCTION: fill_with_char
   INPUT: str - A pointer to the string array that needs to be filled with characters
          c - The character to be filled in the string array
          n - The number of characters to be filled in the string array

    The function uses the "memset" function from the string.h library. There are two calls to "memset" inside the function. The first call fills the "str" string pointer with "c" character n times. The second call adds a null terminator at the end of the "str" string pointer.
*/
char *fill_with_char(char *str, char c, size_t n)
{
    memset(str, c, n);
    memset(str + n, '\0', 1);
    return str;
}

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

    The function uses two nested loops to process all lines and columns. For each column and each line, it computes the number of words and spaces in the incoming string, and based on this information, it makes the necessary adjustments to justify the text. If the line does not fill the whole column, it pads it with spaces until the end of the column is reached.
*/

Pr_data process_one_line(int n_cols, int col_width, int n_rows, int spacing, char **out_lines, Pr_data pos_data)
{
    for (int j = pos_data.j; j < n_cols; j++)
    {
        for (int i = pos_data.i; i < n_rows; i++)
        {
            char *ln_ptr = pos_data.line_ptr;
            int space_cnt = 0;
            int word_cnt = 0;
            int char_cnt = 0;
            while (*ln_ptr != '\0' && char_cnt < col_width)
            {
                if (*ln_ptr == ' ')
                {
                    space_cnt++;
                }
                ln_ptr++;
                if (is_ascii(*ln_ptr))
                    char_cnt++; // do not increment if the byte do not begins with 10
            }
            /* ln_ptr at the end points \0 or one character beyond the max width of the column.

            1. if it points '\0' the paragraph is ended and we need to fill with zeros up to the next column.
            */
            if (*ln_ptr == '\0')
            {
                // check not to be in the first line of a column so as not to add the empty line due to \n or not to be left with an empty string in case it is exactly multiple of the column size.
                if ((i != 0 || strcmp(pos_data.line_ptr, "\n") != 0) && strcmp(pos_data.line_ptr, "") != 0)
                {
                    // append the remainder of the paragraph to the current line, strcat is safe because we have already seen that line is shorter than the column and therefore it will necessarily be shorter than the allocated row
                    strcat(out_lines[i], pos_data.line_ptr);

                    // the -1 is to remove '\n'
                    fill_with_char(out_lines[i] + strlen(out_lines[i]) - 1, ' ', col_width - char_cnt + 1);
                    // if not in the last column add the space between the columns
                    if (j != n_cols - 1)
                        fill_with_char(out_lines[i] + strlen(out_lines[i]), ' ', spacing);

                    // before returning, identify the point from which to resume with the new line
                    if (i < n_rows - 1)
                    { // go to the next line
                        pos_data.i = i + 1, pos_data.j = j;
                    }
                    else if (j < n_cols - 1)
                    { // if reached the last row and not in the last column i go to the next column
                        pos_data.i = 0, pos_data.j = j + 1;
                    }
                    else
                    { // the page is ended, restart from 0
                        pos_data.i = 0, pos_data.j = 0;
                    }
                }
                else
                { // restart from "this" line
                    pos_data.i = i, pos_data.j = j;
                }
                pos_data.line_ptr = ln_ptr;
                return pos_data;
            }

            /*  2. The column can end (words are always separated by at most one space) with a letter and ln_ptr
                points  to ' ' or '\n', the text is already justified, the number of words will be equal to the number of spaces + 1 just copy the text.
            */
            if (((*ln_ptr == ' ' || *ln_ptr == '\n')) && (*(ln_ptr - 1) != ' '))
            {
                strncat(out_lines[i], pos_data.line_ptr, ln_ptr - pos_data.line_ptr);
                pos_data.line_ptr = ln_ptr + 1; // restart from the character after the space or \n (in the latter case it will be \0)
                if (j < n_cols - 1)
                {
                    fill_with_char(out_lines[i] + strlen(out_lines[i]), ' ', spacing);
                }
                continue; // go to the next row
            }

            /*
             The column can end with a space and ln_ptr pointing to any character, then the number of words will be equal to the number of spaces and the last one must be moved before the last word, in fact this case is handled without problems by the general case
            */

            /*
            3. The column can end with a letter and ln_ptr points to a character other than ' ' or '\n' (already
                verified in the previous ifs), then the number of words is equal to the number of spaces and I have to distribute between the words a number of spaces equal to the number of characters counted back to the previous space + the number of words - 1.
             */
            word_cnt = space_cnt;  // correct even if it ends with a space
            ln_ptr--;              // move back the pointer to the current position
            while (*ln_ptr != ' ') // count the characters to replace with spaces
            {
                if (is_ascii(*ln_ptr))
                    space_cnt++; // count only if ascii (the extra utf-8's byte is not visualized)
                ln_ptr--;
            }

            int spc_bw = 1; // the space between words is at least 1 character
            if (word_cnt > 1)
                spc_bw = space_cnt / (word_cnt - 1);
            int spc_ex = 0; // extra space if I don't have integer division
            if (word_cnt > 1)
                spc_ex = space_cnt % (word_cnt - 1);

            // copy the string inserting the spaces in the right place
            char_cnt = 0;
            char *dst = out_lines[i] + strlen(out_lines[i]); // copy the characters from the end of the already stored string
            for (int iw = 0; iw < word_cnt - 1; iw++)
            {
                while ((*dst++ = *pos_data.line_ptr++) != ' ')
                    char_cnt++;
                dst--; // discard the last character read
                memset(dst, ' ', spc_bw);
                dst += spc_bw;
                if (iw == word_cnt - 2)
                { // any extra space is added before the last word
                    memset(dst, ' ', spc_ex);
                    dst += spc_ex;
                }
            }
            // copy the last (or the only) word
            while ((*dst++ = *pos_data.line_ptr++) != ' ')
                char_cnt++;
            dst--;
            if (word_cnt == 1) // if just one word, left align
            {
                memset(dst, ' ', col_width - char_cnt);
                dst += col_width - char_cnt;
            }
            memset(dst, '\0', 1);
            if (j < n_cols - 1) // if it's not the last column add spaces between columns
            {
                fill_with_char(dst, ' ', spacing);
            }
        }
        pos_data.i = 0; // after the first loop it must restart from 0
    }

    // done, the next call must start from 0,0 while the pointer to the line is already updated to the correct location
    pos_data.i = 0;
    pos_data.j = 0;
    return pos_data;
}

/* FUNCTION copy_word
   INPUT: a pointer to a destination character array (dst)
          a pointer to a source character array (src).
   OUTPUT: void (it changes the dst string)
   This function copies a single word from the source array to the destination array, ensuring that the copied word is null-terminated.
*/
void copy_word(char *dst, char *src)
{
    while ((*dst++ = *src++) != ' ')
        ;
    *dst = '\0';
}
