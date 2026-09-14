// 4.c — Multiple fork() calls: 2^n processes
#include <stdio.h>
#include <unistd.h>

int main() {
    printf("Start: PID = %d\n", getpid());

    fork();
    fork();

    printf("PID = %d, PPID = %d\n", getpid(), getppid());

    return 0;
}