// 7.c — Child exit status: wait(&status) + WIFEXITED + WEXITSTATUS
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main() {

    int pid = fork();

    if (pid == 0) {
        printf("Child: Starting\n");

        sleep(2);

        printf("Child: Exiting\n");
        exit(5);
    }
    else {
        int status;

        printf("Parent: Waiting...\n");

        wait(&status);

        printf("Parent: Child finished\n");
        printf("Raw status = %d (encoded — don't use directly)\n", status);

        if (WIFEXITED(status)) {
            printf("Child exited normally with code = %d\n", WEXITSTATUS(status));
        }
        else {
            printf("Child did not exit normally\n");
        }
    }

    return 0;
}