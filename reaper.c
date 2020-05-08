#include "reaper.h"

/*
 * Function that only executes if execv fails. Prints out relevant message.
 */
void handle_execv_errors(char *arg[]) {
    if (errno != ENOENT) {
        fprintf(stderr, "sh: %s failed: %s\n", arg[0], strerror(errno));
        fprintf(stdout, "\n");
    } else {
        fprintf(stderr, "sh: command not found: %s\n", arg[0]);
        fprintf(stdout, "\n");
    }
    exit(1);
}

/*
 * Wait for foreground process(es). If the process
 * terminates normally, return. Otherwise, it is either
 * suspended (and hence added to the jobs list)
 * or terminated. Updates job list
 * when existing fg proc is suspended.
 */
void reap_foreground(job_list_t *job_list, int *jid_pointer, pid_t pid,
                     int status, char command[]) {
    char *term_string = "[%d] (%d) terminated by signal %d\n";
    char *stop_string = "[%d] (%d) suspended by signal %d\n";
    while (waitpid(pid, &status, WUNTRACED) > 0) {
        if (WIFEXITED(status)) {
            // remove job from list if it started in the bg,
            // was brought to the fg and exited normally
            if (get_job_pid(job_list, *jid_pointer) == pid) {
                remove_job_jid(job_list, *jid_pointer);
            }
            return;
        }

        if (WIFSIGNALED(status)) {
            // remove job from list if it started in the background,
            // was brought to the foreground and terminated by a signal
            if (get_job_pid(job_list, *jid_pointer) == pid) {
                if (fprintf(stdout, term_string, *jid_pointer, pid,
                            WTERMSIG(status)) < 0) {
                    perror("fprintf");
                    cleanup_job_list(job_list);
                    exit(1);
                }
                remove_job_jid(job_list, *jid_pointer);
                return;
            }
            // create new job id but do not add proc to job list as it
            // terminates
            else {
                (*(jid_pointer))++;
                if (fprintf(stdout, term_string, *jid_pointer, pid,
                            WTERMSIG(status)) < 0) {
                    perror("fprintf");
                    cleanup_job_list(job_list);
                    exit(1);
                }
                return;
            }
        }

        if (WIFSTOPPED(status)) {
            // check if process already exists, update job in list
            if (get_job_pid(job_list, *jid_pointer) == pid) {
                if (fprintf(stdout, stop_string, *jid_pointer, pid,
                            WSTOPSIG(status)) < 0) {
                    perror("fprintf");
                    cleanup_job_list(job_list);
                    exit(1);
                }
                update_job_jid(job_list, *jid_pointer, STOPPED);
                return;

            } else {
                // new job being created by suspending fg proc
                (*(jid_pointer))++;
                if (fprintf(stdout, stop_string, *jid_pointer, pid,
                            WSTOPSIG(status)) < 0) {
                    perror("fprintf");
                    cleanup_job_list(job_list);
                    exit(1);
                }
                add_job(job_list, *jid_pointer, pid, STOPPED, command);
                return;
            }
            return;
        }
    }
}

/*
 * Listens for change of state of  background processes.
 * If a process has terminated normally, or by a signal,
 * shell should print corresponding message
 * and reap it. If a process is stopped or continued, just print message.
 */
void reap_background(job_list_t *job_list) {
    if (job_list == NULL) {
        return;
    }
    pid_t pid;
    int jid;
    int status;
    char *exit_string = "[%d] (%d) terminated with exit status %d\n";
    char *term_string = "[%d] (%d) terminated by signal %d\n";
    char *stop_string = "[%d] (%d) suspended by signal %d\n";
    char *resumed_string = "[%d] (%d) resumed\n";
    while ((pid = get_next_pid(job_list)) != -1) {
        jid = get_job_jid(job_list, pid);
        while (waitpid(pid, &status, WNOHANG | WCONTINUED | WUNTRACED) > 0) {
            // remove job if term by a signal
            if (WIFSIGNALED(status)) {
                if (fprintf(stdout, term_string, jid, pid, WTERMSIG(status)) <
                    0) {
                    perror("fprintf");
                    cleanup_job_list(job_list);
                    exit(1);
                }
                remove_job_jid(job_list, jid);
                return;
            }
            // update job list if paused by a signal
            if (WIFSTOPPED(status)) {
                if (fprintf(stdout, stop_string, jid, pid, WSTOPSIG(status)) <
                    0) {
                    perror("fprintf");
                    cleanup_job_list(job_list);
                    exit(1);
                }
                update_job_pid(job_list, pid, STOPPED);
                return;
            }
            // update job list if resumed by a signal
            if (WIFCONTINUED(status)) {
                if (fprintf(stdout, resumed_string, jid, pid) < 0) {
                    perror("fprintf");
                    cleanup_job_list(job_list);
                    exit(1);
                }
                update_job_jid(job_list, jid, RUNNING);
                return;
            }
            // remove job from the list once it terminates normally
            if (WIFEXITED(status)) {
                if (fprintf(stdout, exit_string, jid, pid,
                            WEXITSTATUS(status)) < 0) {
                    perror("fprintf");
                    cleanup_job_list(job_list);
                    exit(1);
                }
                remove_job_jid(job_list, jid);
                return;
            }
        }
    }
}