#include "built_in.h"
#include "reaper.h"

int check_cd(char *argv[]) {  // check if exe is "cd" commands
    if (!strcmp(argv[0], "cd")) {
        if (argv[1] != 0) {
            int ch_error =
                chdir(argv[1]);  // stores error output if chdir fails
            if (ch_error != 0) {
                perror("cd");
            }
        } else {
            fprintf(stderr, "cd: syntax error\r\n");
        }

        return 1;
    }
    return 0;
}

int check_ln(char *argv[]) {  // check if exe is "cd" commands
    if (!strcmp(argv[0], "ln")) {
        if (argv[1] == 0) {
            fprintf(stderr,
                    "Missing first argument for ln\r");  // stores error output
                                                         // if missing
        } else if (argv[2] == 0) {  // arguments or link fails
            fprintf(stderr, "Missing second argument for ln\r\n");
        } else {
            int ln_error = link(argv[1], argv[2]);
            if (ln_error != 0) {
                perror("ln");
            }
        }
        return 1;
    }
    return 0;
}

int check_exit(char *argv[]) {  // check if exe is "exit" commands
    if (!strcmp(argv[0], "exit")) {
        if (argv[1] != 0) {
            fprintf(stderr, "Exit: syntax error\r\n");
            // return true if exit fails
            return 1;
        } else {
            return 1;
        }
    }
    return 0;
}

int check_rm(char *argv[]) {  // check if exe is "rm" command
    if (!strcmp(argv[0], "rm")) {
        if (argv[1] != 0) {
            int rm_error =
                unlink(argv[1]);  // stores error output if unlink fails
            if (rm_error != 0) {
                perror("rm");
            }
        } else {
            fprintf(stderr, "rm: Missing argument for removal\r\n");
        }
        return 1;
    }
    return 0;
}

/*
 * Check if jobs is called. Return 1 if so. O/w return 0.
 */
int check_jobs(char *argv[], job_list_t *job_list) {
    if (!strcmp(argv[0], "jobs")) {
        jobs(job_list);
        return 1;
    }

    return 0;
}

/*
 * Check if fg called. Resumes job (if suspended) in the fg.
 */
int check_fg(char *argv[], job_list_t *job_list) {
    if (!strcmp(argv[0], "fg")) {
        if (argv[1] == 0) {
            fprintf(stderr, "fg: syntax error\r\n");
            return 1;
        }
        char arg_cpy[BUFFER_SIZE] = {0};
        int jid;
        int status = 0;
        pid_t pid;
        strncpy(arg_cpy, argv[1], strlen(argv[1]));
        size_t spn = strspn(arg_cpy, "%%");
        char *token = strtok(arg_cpy, "%%");
        if (spn == 0) {
            fprintf(stderr, "Missing percent character in call to fg\r\n");
            return 1;
            // check for valid input
        } else {
            if (!token) {
                fprintf(stderr, "job not found\r\n");
                return 1;
            } else if ((jid = atoi(token)) == 0) {
                fprintf(stderr, "Argument is not an int.\r\n");
                return 1;
            } else if ((pid = get_job_pid(job_list, jid)) == -1) {
                fprintf(stderr, "job not found\r\n");
                return 1;
            } else {
                // take control of terminal to fg proc
                // send continue signal
                // reap process once finished, and
                // take control of terminal back to shell
                tcsetpgrp(STDIN_FILENO, pid);
                kill(-pid, SIGCONT);
                reap_foreground(job_list, &jid, pid, status, argv[0]);
                tcsetpgrp(STDIN_FILENO, getpgrp());
                return 1;
            }
        }
    }

    return 0;
}

/*
 * Check if bg called. Resumes job (if suspended) in the bg.
 */
int check_bg(char *argv[], job_list_t *job_list) {
    if (!strcmp(argv[0], "bg")) {
        // check if argument exists
        if (argv[1] == 0) {
            fprintf(stderr, "bg: syntax error\r\n");
            return 1;
        }
        char arg_cpy[BUFFER_SIZE] = {0};
        int jid;
        pid_t pid;
        strncpy(arg_cpy, argv[1], strlen(argv[1]));
        size_t spn = strspn(arg_cpy, "%%");
        char *token = strtok(arg_cpy, "%%");
        if (spn == 0) {
            fprintf(stderr, "Missing percent character in call to bg\r\n");
            return 1;
        } else {
            // check for valid input
            if (!token) {
                fprintf(stderr, "job not found\r\n");
                return 1;
            } else if ((jid = atoi(token)) == 0) {
                fprintf(stderr, "Argument is not an int.\r\n");
                return 1;
            } else if ((pid = get_job_pid(job_list, jid)) == -1) {
                fprintf(stderr, "job not found\r\n");
                return 1;
            } else {
                kill(-pid, SIGCONT);
                return 1;
            }
        }
    }

    return 0;
}