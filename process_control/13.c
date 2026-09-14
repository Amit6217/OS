// 13.c — Zombie process: multiple children exit, parent doesn't call wait()
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {

    int i;

    for (i = 0; i < 5; i++) {

        int pid = fork();

        if (pid == 0) {
            printf("Child %d (PID = %d): exiting now\n", i + 1, getpid());
            exit(0);  // child exits immediately → becomes zombie
        }
    }

    // Only the parent reaches here
    printf("\nParent (PID = %d): created 5 children\n", getpid());
    printf("Parent: sleeping for 30 seconds WITHOUT calling wait()...\n");
    printf("Run 'ps -e -o pid,ppid,stat,comm | grep Z' in another terminal\n");
    printf("You should see 5 zombie (Z) processes\n\n");

    sleep(30);

    printf("Parent: exiting (all zombies will be cleaned up)\n");

    return 0;
}
