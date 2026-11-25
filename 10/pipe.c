#include "csapp.h"
#include <ctype.h>

#define MAXLINE_LOCAL 1024
#define MAXARGS 32

/* 去掉首尾空白，返回修剪后的起始地址 */
static char *trim(char *s)
{
	while (*s && isspace((unsigned char)*s)) {
		s++;
	}

	if (*s == '\0') {
		return s;
	}

	char *end = s + strlen(s) - 1;
	while (end > s && isspace((unsigned char)*end)) {
		*end = '\0';
		end--;
	}

	return s;
}

/* 把命令按空白分割成 argv，返回参数个数 */
static int parse_argv(char *cmd, char **argv)
{
	int argc = 0;
	char *saveptr = NULL;
	char *tok = strtok_r(cmd, " \t\n", &saveptr);

	while (tok != NULL && argc < MAXARGS - 1) {
		argv[argc++] = tok;
		tok = strtok_r(NULL, " \t\n", &saveptr);
	}

	argv[argc] = NULL;
	return argc;
}

static void run_single_command(char *cmd)
{
	char *argv[MAXARGS];
	int argc = parse_argv(cmd, argv);

	if (argc == 0) {
		return;
	}

	pid_t pid = Fork();
	if (pid == 0) {
		execvp(argv[0], argv);
		fprintf(stderr, "execvp failed: %s\n", argv[0]);
		exit(1);
	}

	Waitpid(pid, NULL, 0);
}

/* 执行 left | right */
static void run_pipeline(char *left_cmd, char *right_cmd)
{
	char *left_argv[MAXARGS];
	char *right_argv[MAXARGS];

	if (parse_argv(left_cmd, left_argv) == 0 || parse_argv(right_cmd, right_argv) == 0) {
		fprintf(stderr, "Invalid command. Example: ls -l | wc -l\n");
		return;
	}

	int pfd[2];
	if (pipe(pfd) < 0) {
		perror("pipe");
		return;
	}

	pid_t left_pid = Fork();
	if (left_pid == 0) {
		/* 子进程1：stdout -> 管道写端 */
		Dup2(pfd[1], STDOUT_FILENO);
		Close(pfd[0]);
		Close(pfd[1]);

		execvp(left_argv[0], left_argv);
		fprintf(stderr, "execvp failed: %s\n", left_argv[0]);
		exit(1);
	}

	pid_t right_pid = Fork();
	if (right_pid == 0) {
		/* 子进程2：stdin <- 管道读端 */
		Dup2(pfd[0], STDIN_FILENO);
		Close(pfd[1]);
		Close(pfd[0]);

		execvp(right_argv[0], right_argv);
		fprintf(stderr, "execvp failed: %s\n", right_argv[0]);
		exit(1);
	}

	/* 父进程关闭管道两端，并等待两个子进程 */
	Close(pfd[0]);
	Close(pfd[1]);
	Waitpid(left_pid, NULL, 0);
	Waitpid(right_pid, NULL, 0);
}

int main(void)
{
	char line[MAXLINE_LOCAL];

	printf("Simple pipe demo (input: A | B, type 'exit' to quit)\n");
	while (1) {
		printf("pipe-demo> ");
		fflush(stdout);

		if (fgets(line, sizeof(line), stdin) == NULL) {
			break;
		}

		if (line[0] == '\n') {
			continue;
		}

		line[strcspn(line, "\n")] = '\0';
		char *input = trim(line);

		if (strcmp(input, "exit") == 0 || strcmp(input, "quit") == 0) {
			break;
		}

		char *sep = strchr(input, '|');
		if (sep == NULL) {
			/* 没有管道时，也允许执行单条命令，便于对比 */
			run_single_command(input);
			continue;
		}

		*sep = '\0';
		char *left = trim(input);
		char *right = trim(sep + 1);

		run_pipeline(left, right);
	}

	return 0;
}

