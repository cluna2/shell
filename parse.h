#include "libs.h"

void cpy_arg_to_dir(char dest[], char *src, size_t length);
int check_parse_int_error(int parse_int, char *parse_error);
int parse(char *inputs[], int num_inputs, char *argv[], char *redir[],
          int *append_check);
