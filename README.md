# SPLIT TEXT

## NAME

> split_text – convert input text to a given number of columns and rows per page

### SYNOPSIS

> split_text [-mv] [-c number] [-l number] [-w number] [-s number]

### DESCRIPTION

split_text is a C program that transforms a text in Italian from one column to multiple columns on multiple pages (such as for a newspaper page). It is possible to choose between a single-process version and a multi-process version with three concurrent processes.

### INPUT

A text file in Unicode (UTF-8) encoding containing a text in Italian structured in paragraphs separated by one or more empty lines (string containing only the character '\n'). Each paragraph consists of one or more strings terminated by '\n' (the input text is all in one column).

### OUTPUT

An ASCII text file containing the input text in columns and layout according to the parameters expressed by the user.

- Pages are separated by the string “\n %%% \n”;
- each paragraph is separated by an empty line;
- the first column of text is on the left;
- all lines, except the last one, of each paragraph are aligned with both margins of the column;
- the last line of each paragraph is only aligned to the left;
- words in a line are separated by at least one space character;
- hyphenation of too long words is not implemented, if the program finds such a word will terminate.

The options are as follows:

    -h  Display this help and exit.

    -m  Uses three processes.

    -v  Display the values used to format the output text.

    -c number
        Number of columns. Defaults to 3

    -l number
        Number of rows per page. Defaults to 47

    -w number
        Width of a column (number of visible characters). Defaults to 22

    -s number
        Number of space characters between columns. Defaults to 10

### EXIT STATUS

The split_text utility exits 0 on success, and >0 if an error occurs.

### EXAMPLES

To convert the file sample_input.txt into the file sample_output.txt having four columns of width 21 separated by 5 spaces and with five rows per page:

> $ ./split_text -m -c 4 -l 5 -w 21 -s 5 < inputs/sample_input.txt > sample_output.txt

### SOURCE FILES

- main.c  
contains the main() function that serves as a starting point for running the program. It processes the user inputs and controls the execution of the program by directing calls to the other functions of the program.  

- processing.c/h  
contains functions used to process and convert the input text.  

- io_utils.c/h  
contains the function that reads the input data.  

- alloc_utils.c/h  
contains helper functions used to deal with arrays and buffers.
