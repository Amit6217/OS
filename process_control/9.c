// 9.c — fork() + exec(): the classic Unix pattern (like a shell)
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {

    int pid = fork();

    if (pid == 0) {

        printf("Child: Running ls...\n");

        execlp("ls", "ls", "-l", NULL);

        printf("Child: exec failed\n");
    }

    else {

        printf("Parent: Waiting for child...\n");

        wait(NULL);

        printf("Parent: Child finished.\n");
    }

    return 0;
}