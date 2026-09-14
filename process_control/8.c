// 8.c — exec(): replaces the current program (no new process)
#include <stdio.h>
#include <unistd.h>

int main() {

    printf("Before exec\n");

    execlp("ls", "ls", "-l", NULL);

    printf("After exec\n");

    return 0;
}
