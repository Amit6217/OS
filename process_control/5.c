// 5.c — wait(): parent waits for child to finish
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {

    int pid = fork();

    if (pid == 0) {
        printf("Child: I am working...\n");
        sleep(3);
        printf("Child: Finished!\n");
    }
    else {
        printf("Parent: Waiting for child...\n");
        wait(NULL);
        printf("Parent: Child has finished.\n");
    }

    return 0;
}