#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 64

int main() {
    char line[MAX_LINE];
    char *args[MAX_ARGS];

    while (1) {
        printf("MyShell>>> ");
        fflush(stdout);

        if (!fgets(line, MAX_LINE, stdin)) {
            break; 
        }

        size_t length = strlen(line);
        if (length > 0 && line[length - 1] == '\n') {
            line[length - 1] = '\0';
        }

        if (strcmp(line, "exit") == 0) {
            break;
        }

        if (strlen(line) == 0) {
            continue;
        }

        int i = 0;
        char *token = strtok(line, " \t");
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " \t");
        }
        args[i] = NULL;

        if (args[0] == NULL) {
            continue;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("Fork failed");
            exit(1);
        } else if (pid == 0) {
            if (execvp(args[0], args) == -1) {
                perror("Command execution failed");
            }
            exit(1);
        } else {
            wait(NULL);
        }
    }

    return 0;
}
