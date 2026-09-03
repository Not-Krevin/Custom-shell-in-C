#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

static int parse_line(char *line, char *argv[], int max_args) {
    int argc = 0;
    char *token = strtok(line, " \t\r\n");

    while (token != NULL && argc < max_args - 1) {
        argv[argc++] = token;
        token = strtok(NULL, " \t\r\n");
    }

    argv[argc] = NULL;
    return argc;
}

static int run_builtin(char *argv[]) {
    if (strcmp(argv[0], "exit") == 0) {
        exit(EXIT_SUCCESS);
    }

    if (strcmp(argv[0], "cd") == 0) {
        const char *target = argv[1];

        if (target == NULL) {
            target = getenv("HOME");
        }

        if (target == NULL) {
            fprintf(stderr, "cd: HOME is not set\n");
            return 1;
        }

        if (chdir(target) == -1) {
            perror("cd");
            return 1;
        }

        return 1;
    }

    return 0;
}

static void run_external(char *argv[]) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork");
        return;
    }

    if (pid == 0) {
        execvp(argv[0], argv);

        perror(argv[0]);
        _exit(127);
    }

    int status;

    while (waitpid(pid, &status, 0) == -1) {
        if (errno == EINTR) {
            continue;
        }

        perror("waitpid");
        return;
    }

    if (WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);

        if (exit_code != 0) {
            fprintf(stderr, "process exited with status %d\n", exit_code);
        }
    } else if (WIFSIGNALED(status)) {
        fprintf(stderr,
                "process terminated by signal %d\n",
                WTERMSIG(status));
    }
}

int main(void) {
    char line[MAX_LINE];
    char *argv[MAX_ARGS];

    while (1) {
        printf("minish> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            if (feof(stdin)) {
                putchar('\n');
                break;
            }

            perror("fgets");
            continue;
        }

        int argc = parse_line(line, argv, MAX_ARGS);

        if (argc == 0) {
            continue;
        }

        if (run_builtin(argv)) {
            continue;
        }

        run_external(argv);
    }

    return EXIT_SUCCESS;
}
