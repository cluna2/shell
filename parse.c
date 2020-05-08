#include "parse.h"

/*
 * Copies argv[0] to command
 */
void cpy_arg_to_dir(char dest[], char *src, size_t length) {
    int c = 0;
    while (c < (int)length) {
        dest[c] = src[c];
        c++;
    }
    dest[c] = '\0';
}

/*
 * Prints error message for parsing. Returns non-zero value if so.
 */
int check_parse_int_error(int parse_int, char *parse_error) {
    int err_right = -1;
    int err_left = -2;
    int err_filename = -3;

    if (parse_int < 0) {  // Checks if parse_int returns error
        if (parse_int == err_right) {
            parse_error = "Error concerning > or >> redirection\n";
        } else if (parse_int == err_left) {
            parse_error = "Error concerning < redirects\n";
        } else if (parse_int == err_filename) {
            parse_error =
                "Error: Redirection symbol received but no program called.\n";
        }
        fprintf(stderr, "%s", parse_error);
        return 1;
    }
    return 0;
}

/*
 * Parses the inputs array and creates the argv array. Returns the number
 * of redir elements found if successful, and returns error variables if it
 * fails.
 */
int parse(char *inputs[], int num_in, char *argv[], char *redir[],
          int *append_check) {
    int pointer = 0;
    int count = 0;
    int has_right = 0;
    int has_left = 0;
    int err_right = -1;
    int err_left = -2;
    int err_filename = -3;
    int filename = 0;

    for (int i = 0; i < num_in; i++) {
        if (has_right &&
            !strcmp(inputs[i], ">")) {  // invalid command-line input check
            return err_right;
        } else if (has_right && !strcmp(inputs[i], ">>")) {
            return err_right;
        } else if (has_left && !strcmp(inputs[i], "<")) {
            return err_left;
        } else if (!has_left && !strcmp(inputs[i], "<")) {
            if (*(inputs[i + 1]) == '>' || *(inputs[i + 1]) == '<' ||
                i >= num_in - 1) {
                return err_left;
            }
            i++;
            redir[1] = inputs[i];  // set redirection array to that element
            count += 2;            // record number of redirecion elements
            has_left =
                1;  // next element has left element related to redirection
        } else if (!has_right &&
                   (!strcmp(inputs[i], ">") || !strcmp(inputs[i], ">>"))) {
            // invalid command-line input check
            if (i >= (num_in - 1) || *(inputs[i + 1]) == '>' ||
                *(inputs[i + 1]) == '<') {
                return err_right;
            } else if (!strcmp(inputs[i], ">>")) {
                *append_check =
                    0;  // stores truncation symbol for >> redirecion
            } else {
                *append_check = 1;
            }

            count += 2;  // record number of redirecion elements
            has_right =
                1;  // next element has right element related to redirection

            i++;
            redir[0] = inputs[i];  // set redirection array to that element
        } else {
            argv[pointer] = inputs[i];  // create argv array
            pointer++;                  // increments pointer for argv creation
            filename = 1;               // filename exists
        }
    }

    if (!filename) {  // check if filename exists next to redirection symbol
        return err_filename;
    }

    return count;  // return number of redir elements found
}
