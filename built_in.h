#include "jobs.h"
#include "libs.h"

int check_cd(char *argv[]);
int check_ln(char *argv[]);
int check_exit(char *argv[]);
int check_rm(char *argv[]);
int check_jobs(char *argv[], job_list_t *job_list);
int check_fg(char *argv[], job_list_t *job_list);
int check_bg(char *argv[], job_list_t *job_list);