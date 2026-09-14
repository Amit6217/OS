// 3.c — Parent and child have separate memory (COW)
#include <stdio.h>
#include <unistd.h>

int main() {
    int x = 10;

    fork();

    x++;

    printf("PID = %d, x = %d\n", getpid(), x);

    return 0;
}