// 1.c — Basic fork(): creates a child process
#include <stdio.h>
#include <unistd.h>

int main() {
    printf("Before fork: PID = %d\n", getpid());

    fork();

    printf("After fork: PID = %d\n", getpid());

    return 0;
}