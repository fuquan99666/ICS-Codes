/* $begin shellmain */
#include "csapp.h"
#define MAXARGS   128
#define MAXJOBS   128

/* for simplicity, we assume a job is just one process */
typedef struct {
    pid_t pid; // Process ID
    int bg;    // 1 if background job, 0 if foreground job
    int running; // 1 if running, 0 if stopped
    char cmdline[MAXLINE]; // Command line
} job_t;

sigset_t chld_mask;
volatile sig_atomic_t fgpid;
pid_t shell_pgid;
job_t jobs[MAXJOBS];

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);

void sigchld_handler(int sig);
void add_job(pid_t pid, int bg, const char *cmdline);
void delete_job(pid_t pid);
void waitfg(pid_t pid);

void delete_job(pid_t pid)
{
    int i;
    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == pid) {
            jobs[i].pid = 0;
            jobs[i].bg = 0;
            jobs[i].running = 0;
            jobs[i].cmdline[0] = '\0';
            return;
        }
    }
}

void add_job(pid_t pid, int bg, const char *cmdline)
{
    int i;
    for (i = 0; i < MAXJOBS; i++) {
        if (jobs[i].pid == 0) {
            jobs[i].pid = pid;
            jobs[i].bg = bg;
            jobs[i].running = 1;
            strncpy(jobs[i].cmdline, cmdline, MAXLINE - 1);
            jobs[i].cmdline[MAXLINE - 1] = '\0';
            return;
        }
    }
}

/* 处理不同类型的子进程信号 */
void sigchld_handler(int sig)
{
    int olderrno = errno;
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        int i;

        if (WIFEXITED(status) || WIFSIGNALED(status)) {
            delete_job(pid);
        } else if (WIFSTOPPED(status)) {
            for (i = 0; i < MAXJOBS; i++) {
                if (jobs[i].pid == pid) {
                    jobs[i].running = 0;
                    break;
                }
            }
        } else if (WIFCONTINUED(status)) {
            for (i = 0; i < MAXJOBS; i++) {
                if (jobs[i].pid == pid) {
                    jobs[i].running = 1;
                    break;
                }
            }
        }

        if ((pid_t)fgpid == pid &&
            (WIFEXITED(status) || WIFSIGNALED(status) || WIFSTOPPED(status)))
            fgpid = 0;
    }

    errno = olderrno;
}

void waitfg(pid_t pid)
{
    sigset_t empty;
    sigemptyset(&empty);

    /* 原子地等待前台作业，避免错过 SIGCHLD。 */
    /* 这里的 while 处理是为了防止别的信号干扰，确保处理 fgpid */
    while ((pid_t)fgpid == pid)
        sigsuspend(&empty);

    tcsetpgrp(STDIN_FILENO, shell_pgid);
}

int main()
{
    char cmdline[MAXLINE];

    shell_pgid = getpid();
    if (setpgid(0, shell_pgid) < 0 && errno != EPERM)
        unix_error("setpgid: failed to create shell process group");
    shell_pgid = getpgrp();

    Sigemptyset(&chld_mask);
    Sigaddset(&chld_mask, SIGCHLD);
    Signal(SIGCHLD, sigchld_handler);
    Signal(SIGINT, SIG_IGN);
    Signal(SIGTSTP, SIG_IGN);
    Signal(SIGTTIN, SIG_IGN);
    Signal(SIGTTOU, SIG_IGN); // 当在 waitfg 中，shell 作为非终端前台进程组的成员，试图抢占终端时，会收到 SIGTTOU 信号，默认行为是 STOP
    tcsetpgrp(STDIN_FILENO, shell_pgid);

    while (1) {
        printf("> ");
        Fgets(cmdline, MAXLINE, stdin);
        if (feof(stdin))
            exit(0);

        eval(cmdline);
    }
}
/* $end shellmain */

/* $begin eval */
void eval(char *cmdline)
{
    char *argv[MAXARGS];
    char buf[MAXLINE];
    int bg;
    pid_t pid;
    sigset_t prev;

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);
    if (argv[0] == NULL)
        return;

    if (!builtin_command(argv)) {
        /* fork 前阻塞 SIGCHLD，避免子进程退出时作业表尚未登记。 */
        Sigprocmask(SIG_BLOCK, &chld_mask, &prev);

        if ((pid = Fork()) == 0) {
            // 子进程单独设置进程组，避免被 shell 的信号处理干扰
            setpgid(0, 0);
            // 子进程使用默认信号处理
            Signal(SIGINT, SIG_DFL);
            Signal(SIGTSTP, SIG_DFL);
            Signal(SIGTTIN, SIG_DFL);
            Signal(SIGTTOU, SIG_DFL);
            Signal(SIGCHLD, SIG_DFL);
            Sigprocmask(SIG_SETMASK, &prev, NULL);
            if (execvp(argv[0], argv) < 0) {
                fprintf(stderr, "%s: Command not found.\n", argv[0]);
                exit(127);
            }
        }

        setpgid(pid, pid);
        add_job(pid, bg, cmdline);
        if (!bg) {
            fgpid = pid;
            // 将终端的前台进程组设置为子进程的进程组，这样子进程就可以接收来自终端的信号
            // 而不会干扰其他进程
            tcsetpgrp(STDIN_FILENO, pid);
        }

        Sigprocmask(SIG_SETMASK, &prev, NULL);

        // 在这里之后，子进程，不论是 fg 还是 bg, 信号什么时候到来都无关紧要
        // 对于 fg 进程，如果信号在 waitfg 到来，那么 fgpid 会被设置为0, waitfg会直接返回
        // 如果进入了 waitfg 之后才到来，就会进入正常的 while 判断等待逻辑
        // 总之，加上一个 fgpid 可以确保有一个保底wait机制

        if (!bg)
            waitfg(pid);
        else
            printf("%d %s", pid, cmdline);
    }
    return;
}

int builtin_command(char **argv)
{
    sigset_t prev;
    pid_t pid;
    int i;

    if (!strcmp(argv[0], "quit"))
        exit(0);
    if (!strcmp(argv[0], "&"))
        return 1;
    if (!strcmp(argv[0], "jobs")) {
        for (i = 0; i < MAXJOBS; i++) {
            if (jobs[i].pid != 0) {
                printf("[%d] %s %s\n", jobs[i].pid,
                       jobs[i].running ? "Running" : "Stopped",
                       jobs[i].cmdline);
            }
        }
        return 1;
    }
    if (!strcmp(argv[0], "fg") || !strcmp(argv[0], "bg")) {
        int foreground = !strcmp(argv[0], "fg");

        if (argv[1] == NULL) {
            printf("%s command requires PID argument\n", argv[0]);
            return 1;
        }
        pid = atoi(argv[1]);

        Sigprocmask(SIG_BLOCK, &chld_mask, &prev);
        for (i = 0; i < MAXJOBS; i++) {
            if (jobs[i].pid == pid) {
                jobs[i].running = 1;
                jobs[i].bg = !foreground;

                if (!foreground)
                    tcsetpgrp(STDIN_FILENO, shell_pgid);

                if (foreground) {
                    fgpid = pid;
                    tcsetpgrp(STDIN_FILENO, pid);
                }

                /* SIGCONT is harmless for an already-running job */
                kill(pid, SIGCONT);

                Sigprocmask(SIG_SETMASK, &prev, NULL);

                if (foreground)
                    waitfg(pid);
                return 1;
            }
        }
        Sigprocmask(SIG_SETMASK, &prev, NULL);

        printf("No such job with PID %d\n", pid);
        return 1;
    }

    return 0;
}
/* $end eval */

/* $begin parseline */
int parseline(char *buf, char **argv)
{
    char *delim;
    int argc;
    int bg;

    buf[strlen(buf) - 1] = ' ';
    while (*buf && (*buf == ' '))
        buf++;

    argc = 0;
    while ((delim = strchr(buf, ' '))) {
        argv[argc++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while (*buf && (*buf == ' '))
            buf++;
    }
    argv[argc] = NULL;

    if (argc == 0)
        return 1;

    if ((bg = (*argv[argc - 1] == '&')) != 0)
        argv[--argc] = NULL;

    return bg;
}
/* $end parseline */


