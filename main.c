#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include "io_utils.h"
#include "processing.h"
#include "alloc_utils.h"

static char new_page[] = "\n %%% \n"; // newpage delimiter

bool process_empty_line(char **line, bool *empty_line, Pr_data pos_data);

void write_one_page(int fd, char **out_lines, int alloc_n_rows);

void mp_main(int n_cols, int n_rows, int spacing, int col_width, int alloc_n_rows, int alloc_page_width);

void sp_main(int n_cols, int n_rows, int spacing, int col_width, int alloc_n_rows, int alloc_page_width);

int main(int argc, char *argv[])
{

    char c_opt;             // for command line input
    int n_cols = 3;         // number of columns
    int spacing = 10;       // number of space characters between columns
    int n_rows = 47;        // number of rows per page
    int col_width = 22;     // width of a column (visible characters)
    bool b_mp = false;      // whether to use multiprocess
    bool b_verbose = false; // whether to print additional information

    // process input from command line
    char help[] = "Usage: split_text [OPTION]... < [FILE]\n\n"
            "The options are as follows:\n\n"
            "-h  Display this help and exit.\n"
            "-m  Uses three processes.\n"
            "-v  Display the values used to format the output text.\n"
            "-c number  Number of columns. Defaults to 3\n"
            "-l number  Number of rows per page. Defaults to 47\n"
            "-w number  Width of a column (number of visible characters). Defaults to 22\n"
            "-s number  Number of space characters between columns. Defaults to 10\n\n"
            "Exit status\n"
            "The split_text utility exits 0 on success, and >0 if an error occurs.\n\n"
            "Example\n"
            "To convert the file sample_input.txt into the file sample_output.txt having four columns of width 21 separated by 5 spaces and with five rows per page:\n"
            "> $ ./split_text -m -c 4 -l 5 -w 21 -s 5 < inputs/sample_input.txt > sample_output.txt\n";

    opterr = 0;
    while ((c_opt = getopt(argc, argv, "hmvc:l:w:s:")) != -1)
        switch (c_opt)
        {
        case 'h':
            fprintf(stdout, help);
            exit(EXIT_SUCCESS);
        case 'm':
            b_mp = true;
            break;
        case 'v':
            b_verbose = true;
            break;
        case 'c':
            n_cols = atoi(optarg);
            if (n_cols < 1)
            {
                fprintf(stderr, "Error: there must be at least 1 column.\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'l':
            n_rows = atoi(optarg);
            if (n_rows < 1)
            {
                fprintf(stderr, "Error: there must be at least 1 row.\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 'w':
            col_width = atoi(optarg);
            if (col_width < 1)
            {
                fprintf(stderr, "Error: column width must be at least 1 character.\n");
                exit(EXIT_FAILURE);
            }
            break;
        case 's':
            spacing = atoi(optarg);
            if (spacing < 1)
            {
                fprintf(stderr, "Error: space between columns must be at least 1.\n");
                exit(EXIT_FAILURE);
            }
            break;
        case '?':
            if (optopt == 'c' || optopt == 'l' || optopt == 'w' || optopt == 's')
                fprintf(stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint(optopt))
                fprintf(stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf(stderr,
                        "Unknown option character `\\x%x'.\n",
                        optopt);
            exit(EXIT_FAILURE);
        default:
            abort();
        }

    if (isatty(STDIN_FILENO)) {
        // L'input NON è stato rediretto
        fprintf(stderr, help);
        exit(EXIT_FAILURE);
    }

    // Compute other useful values
    // allocate extra space to hold accented characters. A line of accented characters would take up twice as much space as regular characters + 1 for '\0'
    int page_width = col_width * n_cols + spacing * (n_cols - 1);
    int alloc_page_width = page_width * 2 + 1;
    if (alloc_page_width < (strlen(new_page) + 1))
    {
        fprintf(stderr, "The width of the page is smaller than the new page symbol, must stop.\n");
        exit(EXIT_FAILURE);
    }
    int alloc_n_rows = n_rows + 1; // one extra line for the newpage symbol

    if (b_verbose)
    {
        printf("Number of columns: %d\n", n_cols);
        printf("Space between columns: %d\n", spacing);
        printf("Page width: %d\n", page_width);
        printf("Number of rows per page: %d\n", n_rows);
        printf("Column width: %d\n", col_width);
    }

    if (b_mp)
    {
        mp_main(n_cols, n_rows, spacing, col_width, alloc_n_rows, alloc_page_width);
    }
    else
    {
        sp_main(n_cols, n_rows, spacing, col_width, alloc_n_rows, alloc_page_width);
    }

    return EXIT_SUCCESS;
}

/* FUNCTION: process_empty_line
 * INPUT:   a pointer to a pointer of char (char** line), the line to be checked
            a pointer to a boolean (bool* empty_line), whether an empty line has been already found
            a struct Pr_data (struct Pr_data pos_data), the position on the page (to determine whether the line is the  frirst of a page)
 * OUTPUT:  boolean value. If the line is empty and it is not the firts time an empty line is found, the function
            returns true (indicating that the line should be discarded). Otherwise, it returns false. In any case the functon updates the value of *empty_line to match what it has found.
 */
bool process_empty_line(char **line, bool *empty_line, Pr_data pos_data)
{
    if (**line == '\0')
    { // if this is the first time that reads an empty row and it is not the first row of a page
        if (!*empty_line && !(pos_data.i == 0 && pos_data.j == 0))
        {
            strcpy(*line, "\n"); // newline whill become a row of spaces
            *empty_line = true;
        }
        else
        { // tell the caller to discard duplicate empty lines
            return true;
        }
    }
    else
    {
        *empty_line = false;
    }

    return false;
}

/*  FUNCTION: write_one_page
    INPUT:  fd - a file descriptor to write to.
            out_lines - an array of character pointers, representing lines of text to write to fd.
            alloc_n_rows - an integer representing the number of lines to write.
    OUTPUT: void

    The function loops through the out_lines array and writes each line, followed by a newline character, to the file descriptor using the write system call. If the number of characters written is not equal to the length of the line specified in the strlen call, the function prints an error message using perror, exits the program with EXIT_FAILURE, and returns control to the calling function.
*/
void write_one_page(int fd, char **out_lines, int alloc_n_rows)
{
    for (int i = 0; i < alloc_n_rows; i++)
    {
        if (out_lines[i][0] == '\0') // no more lines to write
            break;
        if (write(fd, out_lines[i], strlen(out_lines[i])) != strlen(out_lines[i]))
        {
            perror("write error");
            exit(EXIT_FAILURE);
        }
        write(fd, "\n", 1);
    }
}

/*  FUNCTION: mp_main
    INPUT:  n_cols, the number of columns for the output.
            n_rows, the number of rows per page for the output.
            spacing, the number of spaces between columns.
            col_width, the width (number of visible characters) of each column.
            alloc_n_rows, the number of rows per page (including the new page symbol).
            alloc_page_width, the width of a row in memory.
    OUTPUT: void

    This is a function three interconnected processes that takes as input the parameters related to the desired layout for the output text and prints it with a given number of columns and rows per page and a certain spacing between the columns (if the input had empty lines pagination is done properly).

    The code first declares several variables for processing rows, including a char pointer 'line' with an initial value of NULL, and a structure called Pr_data. Then, variables for multiprocess are declared, such as file descriptors for two pipes and two process ids.

    After initializing the first pipe, a fork is made and the parent process reads line by line from the standard input and writes it to the pipe. Then, the parent process closes the write end of the pipe and waits for the (first) child process to finish.

    The child process reads lines from the pipe, processes them, and writes output to a new pipe. It first opens a new pipe and a second fork is performed. The second parent process will read from the first pipe and write to the second. It will allocate a matrix of the size of a page and read from the first pipe. It processes the data and fills pages until there are no words in the line. When the page is ended, the separator is added, and the page array is reset. This process is repeated until all data is processed. Finally, it writes the last page, closes both pipes and waits for children to terminate.

    The second child reads data from the second pipe and writes them onto the standard output. It reads the data until there is no more data and writes to the standard output. Finally, it closes the second pipe, frees the allocated memory, and terminates.
*/
void mp_main(int n_cols, int n_rows, int spacing, int col_width, int alloc_n_rows, int alloc_page_width)
{
    // Variables to prcess rows
    char *line = NULL;
    ssize_t linelen, nbytes;
    Pr_data pos_data = {.line_ptr = line, .i = 0, .j = 0};
    bool empty_line = false;

    // Variabiles for multiprocess
    int fd[2];  // file descriptors for the read and write ends of the first pipe
    pid_t pid1; // the child process id returned by the first fork()

    // Open the first pipe
    if (pipe(fd) == -1)
    {
        perror("Pipe 1 failed");
        exit(EXIT_FAILURE);
    }
    // fork
    pid1 = fork();

    if (pid1 < 0)
    {
        perror("Fork 1 failed");
        exit(EXIT_FAILURE);
    }
    else if (pid1 > 0) // parent process reads line by line from stdin and writes it to the pipe
    {
        // close the unused read end of the pipe
        close(fd[0]);

        while (read_one_line(stdin, &line, col_width) != EOF)
        {
            linelen = strlen(line) + 1;              // + 1 to include the terminating null character
            write(fd[1], &linelen, sizeof(linelen)); // send the size of the line to the child process
            write(fd[1], line, linelen);             // send the line itself to the child process
        }
        // the write is finished, close the write end of the pipe
        close(fd[1]);
        // free the memory allocated by read_one_line
        free(line);
        // wait for the (first) child process to finish
        if (waitpid(pid1, NULL, 0) != pid1)
        {
            perror("waitpid 1 error");
            exit(EXIT_FAILURE);
        }
    }
    else // child process reads lines from pipe, process them, and writes the output to a new pipe
    {
        // close the unused write end of the pipe
        close(fd[1]);

        // variables for second the fork
        pid_t pid2; // the child process id returned by the second fork()
        int fd2[2]; // file descriptors for the read and write ends of the second pipe

        // open a new pipe
        if (pipe(fd2) == -1)
        {
            perror("Pipe 2 failed");
            exit(EXIT_FAILURE);
        }

        // second fork
        if ((pid2 = fork()) < 0)
        {
            perror("Fork 2 failed");
            exit(EXIT_FAILURE);
        }
        else if (pid2 > 0) // second parent process, it will read from the first pipe and write to the second
        {
            // close the unused read end of the second pipe
            close(fd2[0]);

            // allocate a matrix of the size of a page, this matrix will be rewritten every time
            char **out_lines = alloc_2d(alloc_n_rows, alloc_page_width);

            // read from the first pipe
            while (1)
            {
                // read the size of the line to read
                nbytes = read(fd[0], &linelen, sizeof(linelen));
                if (nbytes <= 0) // no more data to read
                    break;
                // actual read
                line = resize_buffer(&line, linelen);
                nbytes = read(fd[0], line, linelen);
                if (nbytes <= 0)
                {
                    perror("Pipe 1 read terminated unexpectedly");
                    exit(EXIT_FAILURE);
                }

                // process the data. The variable pos_data stores the current position of the read buffer and of the output array.
                pos_data.line_ptr = line;
                // skip if more than one empty line is found
                if (process_empty_line(&line, &empty_line, pos_data))
                {
                    continue;
                }
                // fill pages until there are words in the line
                while (strcmp(pos_data.line_ptr, "") != 0)
                {
                    pos_data = process_one_line(n_cols, col_width, n_rows, spacing, out_lines, pos_data);
                    if (pos_data.i == 0 && pos_data.j == 0)
                    {                                        // the page is ended: add the separator and reset
                        strcpy(out_lines[n_rows], new_page); // this is safe because the size is checked at the beginning of the main function
                        for (int i = 0; i <= n_rows; i++)    // write on the second pipe
                            write(fd2[1], out_lines[i], alloc_page_width);
                        // printf("%s\n", out_lines[i]);
                        for (int i = 0; i <= n_rows; i++) // reset the page array
                            out_lines[i][0] = '\0';
                    }
                }
            }
            for (int i = 0; i <= n_rows; i++)
            {                                // write the last page
                if (out_lines[i][0] == '\0') // no more lines to read
                    break;
                write(fd2[1], out_lines[i], alloc_page_width);
            }
            // printf("%s\n", out_lines[i]);
            // close both pipes
            close(fd[0]);
            close(fd2[1]);
            // free memory allocated
            free(line);
            free_2d(out_lines, alloc_n_rows);
            // wait for children to terminate
            if (waitpid(pid2, NULL, 0) != pid2) // wait for second child
                perror("waitpid 2 error");
        }
        else // second child reads data from second pipe and write them on the stdout
        {
            // close unused write end of the second pipe
            close(fd2[1]);
            line = resize_buffer(&line, alloc_page_width);
            // while there are data to read
            while ((nbytes = read(fd2[0], line, alloc_page_width)) > 0)
            {
                if (write(STDOUT_FILENO, line, strlen(line)) != strlen(line))
                {
                    perror("write error");
                    exit(EXIT_FAILURE);
                }
                write(STDOUT_FILENO, "\n", 1);
            }
            // close second pipe
            close(fd2[0]);
            // free allocated memory
            free(line);
        }
    }
}

/*  FUNCTION: sp_main
    INPUT:  n_cols, the number of columns for the output.
            n_rows, the number of rows per page for the output.
            spacing, the number of spaces between columns.
            col_width, the width (number of visible characters) of each column.
            alloc_n_rows, the number of rows per page (including the new page symbol).
            alloc_page_width, the width of a row in memory.
    OUTPUT: void

    This is the single process version of the mp_main funciont. It takes as input the parameters related to the desired layout for the output text and prints it with a given number of columns and rows per page and a certain spacing between the columns (if the input had empty lines pagination is done properly).
*/
void sp_main(int n_cols, int n_rows, int spacing, int col_width, int alloc_n_rows, int alloc_page_width)
{
    // Variables to prcess rows
    char *line = NULL;
    Pr_data pos_data = {.line_ptr = line, .i = 0, .j = 0};
    bool empty_line = false;

    // allocate a matrix of the size of a page, this matrix will be rewritten every time
    char **out_lines = alloc_2d(alloc_n_rows, alloc_page_width);

    while (read_one_line(stdin, &line, col_width) != EOF)
    {
        // process the data. The variable pos_data stores the current position of the read buffer and of the output array.
        pos_data.line_ptr = line;
        // skip if more than one empty line is found
        if (process_empty_line(&line, &empty_line, pos_data))
        {
            continue;
        }
        // fill pages until there are words in the line
        while (strcmp(pos_data.line_ptr, "") != 0)
        {
            pos_data = process_one_line(n_cols, col_width, n_rows, spacing, out_lines, pos_data);
            if (pos_data.i == 0 && pos_data.j == 0)
            { // the page is ended: add the separator and reset
                strcpy(out_lines[n_rows], new_page); // safe because the size has been checked at the beginning 
                write_one_page(STDOUT_FILENO, out_lines, alloc_n_rows);
                for (int i = 0; i < alloc_n_rows; i++) // reset the page array
                    out_lines[i][0] = '\0';
            }
        }
    }
    // write the last page
    write_one_page(STDOUT_FILENO, out_lines, alloc_n_rows);
    // free allocated memory
    free(line);
    free_2d(out_lines, alloc_n_rows);
}