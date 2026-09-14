// 10.c — Nondeterministic scheduling: run multiple times to see different order
#include <stdio.h>
#include <unistd.h>

int main() {

    int pid = fork();

    if (pid == 0) {
        printf("CHILD\n");
    }
    else {
        printf("PARENT\n");
    }

    return 0;
}