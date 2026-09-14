// 2.c — fork() return value: distinguish parent from child
#include <stdio.h>
#include <unistd.h>

int main() {
    int x = fork();

    if (x < 0) {
        perror("fork failed");
        return 1;
    }
    else if (x == 0) {
        printf("I am CHILD. PID = %d, fork returned = %d\n",
               getpid(), x);
    }
    else {
        printf("I am PARENT. PID = %d, child PID = %d\n",
               getpid(), x);
    }

    return 0;
}