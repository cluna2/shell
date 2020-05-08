#include "read.h"

/*
 * Checks if last character read is CTRL-D and exits if so
 * Also checks if user presses ENTER on blank line and prints next prompt
 */
int check_buffer_state(char buffer[], ssize_t bytes_read) {
    // create buffer copy
    char buffer_cpy[BUFFER_SIZE] = {0};
    strncpy(buffer_cpy, buffer, (size_t)bytes_read);

    // / only happens when CTRL-D pressed
    if (bytes_read == 0) {
        return 3;
        // check for bg process
    } else if (buffer_cpy[bytes_read - 2] == '&') {
        return 2;
        // buffer only contains whitespace
    } else if (!strtok(buffer_cpy, " \t\n")) {
        return 1;
    }
    return 0;
}

/*
 * Copies buffer elements into in[], taking out spaces, tabs, and newlines
 */
int read_buffer_into_in(char *in[], char buffer[]) {
    const char delim[4] = " \n\t";
    int pointer = 0;
    char *token = strtok(buffer, delim);
    // buffer somehow contains only whitespace
    if (!token) {
        return -1;
        // buffer only contains "&"
    } else if (!strcmp(token, "&")) {
        return -2;
    }
    while (token) {
        if (!strcmp(token, "&")) {
            break;
        }
        in[pointer] = token;
        token = strtok(NULL, delim);
        pointer++;
    }

    return pointer;  // return number of elements copied
}

/*
 * Instantiates array of directories from argv[0]
 */
int read_dir_arr(char *dirs[], char *arg0) {
    const char delim[2] = "/";
    int pointer = 0;
    char *token = strtok(arg0, delim);
    while (token) {
        dirs[pointer] = token;
        token = strtok(NULL, delim);
        pointer++;
    }
    return pointer;
}
