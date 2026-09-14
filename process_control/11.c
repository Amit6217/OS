// 11.c — wait() forces deterministic ordering (compare with 10.c)
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {

    int pid = fork();

    if (pid == 0) {
        printf("CHILD\n");
    }
    else {
        wait(NULL);
        printf("PARENT\n");
    }

    return 0;
}