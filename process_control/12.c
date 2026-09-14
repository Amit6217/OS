// 12.c — Orphan process: parent exits first, child gets reparented
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main() {

    int pid = fork();

    if (pid == 0) {
        printf("Child: PID = %d, PPID = %d\n", getpid(), getppid());
        sleep(3);  // parent will exit during this time
        printf("Child after sleep: PID = %d, PPID = %d (reparented!)\n",
               getpid(), getppid());
        // PPID will have changed — child is now an orphan
    }
    else {
        printf("Parent (PID = %d): exiting immediately\n", getpid());
        exit(0);
    }

    return 0;
}
