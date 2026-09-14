#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
    printf("Before fork(), PID = %d\n", getpid());

    int rc = fork();

    if (rc < 0) {
        printf("Fork failed\n");
        exit(1);
    }
    else if (rc == 0) {
        // Child process
        printf("Hello, I am the child process.\n");
        printf("Child PID = %d, Parent PID = %d\n", getpid(), getppid());
    }
    else {
        // Parent process
        printf("Hello, I am the parent process.\n");
        printf("Parent PID = %d, Child PID = %d\n", getpid(), rc);
    }

    return 0;
}
