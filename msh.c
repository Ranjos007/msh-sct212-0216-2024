#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_ARGS 128

int main(void)
{
    char *line = NULL;
    size_t cap = 0;

    while (1) {
        printf("msh> ");
        fflush(stdout);
        ssize_t nread = getline(&line, &cap, stdin);
        if (nread == -1) {
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
    }

    free(line);
    return 0;
}
