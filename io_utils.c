#include "io_utils.h"

/*  FUNCTION: read_one_line
    INPUT:  fin, a pointer to an input stream.
            out_line, poitner to the string where to write the processed lines.
            col_width, the width of a column.
    OUTPUT: the number of words in a line (newline considered as a single word) or EOF if fin reached the end.

    Function that reads and process one line from the stream fin and returns a string of words separated by a single space. Empty lines are converted in lines containing only the \n character. Is up to the caller to free out_line and open and close the stream.
*/
ssize_t read_one_line(FILE *fin, char **out_line, const int col_width)
{
    char *line = NULL;
    size_t linecap = 0;
    ssize_t linelen;
    int cnt = 0;

    // linenel contains the number of characters written, excluding the terminating NUL character, linecap the capacity of the buffer
    linelen = getline(&line, &linecap, fin);
    if (*out_line == NULL || strlen(*out_line) < linecap)
    {
        char *tmp_line = realloc(*out_line, linecap); 
        if (tmp_line == NULL)
        {
            perror("Error allocating the line to be processed. Must stop");
            exit(EXIT_FAILURE);
        }
        *out_line = tmp_line;
    }
    strcpy(*out_line, ""); // reset the buffer

    // strspn returns the index of the first character not in the charset. If it returns \0 it means that in the string there are only characters in the charset, so, in this case, we have an empty line.
    if (line[strspn(line, " \t\v\r\n")] != '\0')
    {
        int n;
        char *pline = line;
        char word[linecap];
        while (sscanf(pline, "%s%n", word, &n) == 1)
        {
            if (strlen(word) > col_width) // -1 to account for \0
            {
                fprintf(stderr, "ERROR: read a word (%s) larger (%lu) than a column (%d), must stop.\n", word, strlen(word), col_width);
                exit(EXIT_FAILURE);
            }
            // strat is safe because the buffer has the size of the entire line from which the word is taken
            strcat(*out_line, word); 
            strcat(*out_line, " ");
            pline += n;
            cnt++;
        }
        // remove the last space and add the newline character
        strcpy(*out_line + strlen(*out_line) - 1, "\n\0");
    }
    free(line);

    if (linelen == -1)
        return linelen;
    return cnt;
}