#include "built_in.h"
#include "parse.h"
#include "read.h"
#include "reaper.h"
#include "redirect.h"

int main() {
    char *in[BUFFER_SIZE];       // array of inputs from keyboard
    char buffer[BUFFER_SIZE];    // buffer storing keyboard input
    char *redir[2];              // array of redirection symbols
    char *argv[BUFFER_SIZE];     // argv
    char *dirs[BUFFER_SIZE];     // array of directory entries
    char command[BUFFER_SIZE];   // executable being called
    char *parse_message = '\0';  // parse error message
    ssize_t bytes_read;          // bytes_read from keyboard
    int buffer_state;            // records state of buffer input for early exit
    int num_in;                  // number of arguments to command
    int parse_int;               // number of elements related to redirection
    int parse_error = 0;
    int cd_bool;  // booleans set if built_ins called
    int ln_bool;
    int exit_bool;
    int rm_bool;
    int jobs_bool;
    int fg_bool;
    int bg_bool;
    int num_dir;       // number of directory entries
    int append_check;  // variable finds > or >> symbol
    pid_t pid;         // process id of child
    int status = 0;    // status of executing program
    int is_bg = 0;
    job_list_t *job_list = init_job_list();
    int job_id = 0;

    // ignore signals in shell itself
    signal(SIGTTOU, SIG_IGN);
    if (errno) {
        perror("SIGTTOU ");
        return 1;
    }
    signal(SIGINT, SIG_IGN);
    if (errno) {
        perror("SIGINT ");
        return 1;
    }
    signal(SIGTSTP, SIG_IGN);
    if (errno) {
        perror("SIGTSTP ");
        return 1;
    }
    signal(SIGQUIT, SIG_IGN);
    if (errno) {
        perror("SIGQUIT ");
        return 1;
    }

    while (1) {
        // clear all variables to handle next prompt
        memset(redir, '\0', sizeof(redir));
        memset(in, '\0', sizeof(in));
        memset(argv, '\0', sizeof(argv));
        memset(dirs, '\0', sizeof(dirs));
        memset(buffer, '\0', sizeof(buffer));

        reap_background(job_list);
#ifdef PROMPT
        if (printf("33sh> ") < 0) {
            perror("fprintf");
            cleanup_job_list(job_list);
            return 1;
        }
        fflush(stdout);
#endif
        // read command into buffer
        bytes_read = read(0, buffer, BUFFER_SIZE);
        if (bytes_read < 0 || errno) {
            fprintf(stderr, "Reading from stdin failed. Exiting.");
            cleanup_job_list(job_list);
            return 1;
        }
        buffer_state = check_buffer_state(buffer, bytes_read);

        if (buffer_state == 1) {  // continue loop if ENTER pressed
            reap_background(job_list);
            continue;
        } else if (buffer_state == 2) {  // running bg process
            is_bg = 1;
        } else if (buffer_state == 3) {  // exit if CTRL-D is pressed
            cleanup_job_list(job_list);
            return 0;
        }

        num_in = read_buffer_into_in(in, buffer);
        // continue if ENTER pressed and first check fails or if only "&"
        // pressed
        if (num_in < 0) {
            is_bg = 0;
            reap_background(job_list);
            continue;
        }

        // parse the in array and print error message if invalid
        parse_int = parse(in, num_in, argv, redir, &append_check);
        parse_error = check_parse_int_error(parse_int, parse_message);

        if (parse_error) {
            reap_background(job_list);
            continue;
        } else {
            num_in -= parse_int;
        }

        // copy argv[0] to command
        cpy_arg_to_dir(command, argv[0], strlen(argv[0]));

        // check if any built_in functions are called and execute
        cd_bool = check_cd(argv);
        ln_bool = check_ln(argv);
        exit_bool = check_exit(argv);
        rm_bool = check_rm(argv);
        jobs_bool = check_jobs(argv, job_list);
        fg_bool = check_fg(argv, job_list);
        bg_bool = check_bg(argv, job_list);

        // repeat REPL (or exit) if any of the above were called
        if (cd_bool) {
            continue;
        } else if (ln_bool) {
            continue;
        } else if (exit_bool) {
            cleanup_job_list(job_list);
            return 0;
        } else if (rm_bool) {
            continue;
        } else if (jobs_bool) {
            continue;
        } else if (fg_bool) {
            continue;
        } else if (bg_bool) {
            continue;
        }

        // get number of directories in argv[0]
        num_dir = read_dir_arr(dirs, argv[0]);
        argv[0] = dirs[num_dir - 1];  // sets argv[0] to program name
        if ((pid = fork()) == 0) {
            // give child new process group id
            setpgid(0, 0);

            // make sure terminal sends signals from stdin
            // to the right processes
            // (i.e. the processes of the child's process group)
            if (!is_bg) {
                tcsetpgrp(0, getpgrp());
                if (errno) {
                    perror("tcsetpgrp ");
                    return 1;
                }
            }

            // unmask signals to let child be affected
            signal(SIGTTOU, SIG_DFL);
            if (errno) {
                perror("SIGTTOU");
                return 1;
            }
            signal(SIGINT, SIG_DFL);
            if (errno) {
                perror("SIGINT");
                return 1;
            }
            signal(SIGTSTP, SIG_DFL);
            if (errno) {
                perror("SIGTSTP");
                return 1;
            }
            signal(SIGQUIT, SIG_DFL);
            if (errno) {
                perror("SIGQUIT");
                return 1;
            }

            // check for redirection errors, open files
            redirect_fd(redir, append_check);
            // execute the program
            execv(command, argv);
            // if execv fails, handle and print errors
            handle_execv_errors(argv);
        }
        // wait for child process to finish before printing next prompt
        if (!is_bg) {
            reap_foreground(job_list, &job_id, pid, status, command);

            // be sure to take control of stdin back to shell
            // o/w, shell prematurely exits
            tcsetpgrp(STDIN_FILENO, getpgrp());
            if (errno) {
                perror("tcsetpgrp");
                return 1;
            }

        } else {
            // o/w add bg process to job list
            job_id++;
            add_job(job_list, job_id, pid, RUNNING, command);
            if (fprintf(stdout, "[%d] (%d)\n", job_id, pid) < 0) {
                perror("fprintf");
                cleanup_job_list(job_list);
                return 1;
            }
            is_bg = 0;
        }
    }
    cleanup_job_list(job_list);
    return 0;
}
