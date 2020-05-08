#include "jobs.h"
#include "libs.h"

struct job_element {
    int jid;
    pid_t pid;
    process_state_t state;
    char *command;
    struct job_element *next;
};
typedef struct job_element job_element_t;

struct job_list {
    job_element_t *head;
    job_element_t *current;
    pid_t shell_pid;
};

void reap_foreground(job_list_t *job_list, int *jid_pointer, pid_t pid,
                     int status, char command[]);
void reap_background(job_list_t *job_list);
void handle_execv_errors(char *arg[]);