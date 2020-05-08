#include "redirect.h"

/*
 * Opens the file set for redirection. Prints out error messages if file opening
 * fails and exits the program.
 */
void redirect_fd(char *redir[], int right_check) {
    int open_error = 0;
    int dup2_error = 0;

    /*
     * An output symbol exists if redir[0] is not null. truncate_check is used
     * to see if the symbol is > or >>
     */
    if (redir[0] != 0) {
        if (right_check == 0) {
            // creates a file if it doesn't exist and sets permissions, appends
            // to file
            open_error = open(redir[0], O_CREAT | O_APPEND | O_WRONLY, 0777);
            if (open_error == -1) {  // prints out error message if open fails
                perror("open");
                exit(1);
            }
            // redirects stdout to fd given by open_error
            dup2_error = dup2(open_error, STDOUT_FILENO);
            if (dup2_error == -1) {  // prints out error message if dup2 fails
                perror("dup2");
                exit(1);
            }

        } else if (right_check == 1) {
            // creates a file if it doesn't exist and sets permissions,
            // and truncates file length to zero
            open_error = open(redir[0], O_CREAT | O_TRUNC | O_WRONLY, 0777);
            if (open_error == -1) {
                perror("open");
                exit(1);
            }
            // points stdout to fd given by open_error
            dup2_error = dup2(open_error, STDOUT_FILENO);
            if (dup2_error == -1) {
                perror("dup2");
                exit(1);
            }
        }
    }
    /*
     * An input symbol exists. The file is opened, and points stdin to fd
     */
    if (redir[1] != 0) {
        open_error = open(redir[1], O_RDONLY);
        if (open_error == -1) {
            perror("open");
            exit(1);
        }

        dup2_error = dup2(open_error, STDIN_FILENO);
        if (dup2_error == -1) {
            perror("dup2");
            exit(1);
        }
    }
}
