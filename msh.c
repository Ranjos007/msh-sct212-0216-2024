#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_ARGS 128

static void print_help(void)
{
    printf("msh - minimal Unix shell\n");
    printf("Built-in commands:\n");
    printf("  cd [dir]  Change current directory (defaults to $HOME)\n");
    printf("  help      Show this help message\n");
    printf("  exit      Exit the shell\n");
}

int main(void)
{
    char *line = NULL;
    size_t cap = 0;
    int last_status = 0;

    while (1) {
        printf("msh[%d]> ", last_status);
        fflush(stdout);
        ssize_t nread = getline(&line, &cap, stdin);
        if (nread == -1) { printf("\n"); break; }
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
        if (pid < 0) { perror("msh: fork"); last_status = 1; }
        else if (pid == 0) {
            execvp(argv[0], argv);
            perror("msh");
            _exit(127);
        } else {
            int status;
            if (waitpid(pid, &status, 0) < 0) { perror("msh: waitpid"); last_status = 1; }
            else if (WIFEXITED(status)) last_status = WEXITSTATUS(status);
            else if (WIFSIGNALED(status)) last_status = 128 + WTERMSIG(status);
        }
    }
    free(line);
    return 0;
}
