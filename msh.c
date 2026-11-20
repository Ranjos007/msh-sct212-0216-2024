#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS 128

static volatile sig_atomic_t got_sigint = 0;

static void handle_sigint(int sig)
{
    (void)sig;
    got_sigint = 1;
}

static void reap_zombies(void)
{
    int status;
    while (waitpid(-1, &status, WNOHANG) > 0) {}
}

static void print_help(void)
{
    printf("msh - minimal Unix shell\n");
    printf("Built-in commands:\n");
    printf("  cd [dir]  Change current directory (defaults to $HOME)\n");
    printf("  help      Show this help message\n");
    printf("  exit      Exit the shell\n");
    printf("Features: external commands, background (&), output redirection (>), Ctrl-C handling\n");
}

int main(void)
{
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    char *line = NULL;
    size_t cap = 0;
    int last_status = 0;

    while (1) {
        reap_zombies();
        printf("msh[%d]> ", last_status);
        fflush(stdout);

        got_sigint = 0;
        ssize_t nread = getline(&line, &cap, stdin);
        if (nread == -1) {
            if (got_sigint) {
                clearerr(stdin);
                printf("\n");
                continue;
            }
            printf("\n");
            break;
        }
        if (nread > 0 && line[nread - 1] == '\n') line[nread - 1] = '\0';

        char *argv[MAX_ARGS];
        int argc = 0;
        char *tok = strtok(line, " \t");
        while (tok != NULL && argc < MAX_ARGS - 1) {
            argv[argc++] = tok;
            tok = strtok(NULL, " \t");
        }
        argv[argc] = NULL;
        if (argc == 0) continue;

        int background = 0;
        if (strcmp(argv[argc - 1], "&") == 0) {
            background = 1;
            argv[--argc] = NULL;
            if (argc == 0) continue;
        }

        char *out_file = NULL;
        for (int i = 0; i < argc; i++) {
            if (strcmp(argv[i], ">") == 0) {
                if (i + 1 >= argc) {
                    fprintf(stderr, "msh: syntax error: missing filename after '>'\n");
                    out_file = (char *)-1;
                    break;
                }
                out_file = argv[i + 1];
                argv[i] = NULL;
                argc = i;
                break;
            }
        }
        if (out_file == (char *)-1) { last_status = 1; continue; }
        if (argc == 0) continue;

        if (strcmp(argv[0], "exit") == 0) break;
        if (strcmp(argv[0], "help") == 0) { print_help(); last_status = 0; continue; }
        if (strcmp(argv[0], "cd") == 0) {
            const char *target = (argc > 1) ? argv[1] : getenv("HOME");
            if (!target) { fprintf(stderr, "msh: cd: HOME not set\n"); last_status = 1; }
            else if (chdir(target) != 0) { perror("msh: cd"); last_status = 1; }
            else last_status = 0;
            continue;
        }

        pid_t pid = fork();
        if (pid < 0) { perror("msh: fork"); last_status = 1; continue; }

        if (pid == 0) {
            signal(SIGINT, SIG_DFL);
            if (out_file != NULL) {
                int fd = open(out_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (fd < 0) { perror("msh: open"); _exit(1); }
                if (dup2(fd, STDOUT_FILENO) < 0) { perror("msh: dup2"); close(fd); _exit(1); }
                close(fd);
            }
            execvp(argv[0], argv);
            perror("msh");
            _exit(127);
        } else if (background) {
            printf("[bg] PID %d\n", (int)pid);
            last_status = 0;
        } else {
            int status;
            while (waitpid(pid, &status, 0) < 0) {
                if (errno == EINTR) continue;
                perror("msh: waitpid");
                last_status = 1;
                break;
            }
            if (WIFEXITED(status)) last_status = WEXITSTATUS(status);
            else if (WIFSIGNALED(status)) last_status = 128 + WTERMSIG(status);
        }
    }

    free(line);
    return 0;
}
