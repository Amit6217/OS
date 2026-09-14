#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    printf("Parent Process: Starting... (PID: %d)\n", getpid());

    pid_t pid = fork();

    if (pid < 0) {
        // Fork failed
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process
        printf("Child Process: I am the child! (PID: %d)\n", getpid());
        printf("Child Process: Executing 'ls -l' using execv()...\n");
        
        // Setup arguments for execv
        char *args[] = {"/bin/ls", "-l", NULL};
        
        // Execute the command
        execv(args[0], args);
        
        // execv only returns if an error occurred
        perror("execv failed");
        exit(1);
    } else {
        // Parent process
        printf("Parent Process: Waiting for child (PID: %d) to finish...\n", pid);
        
        int status;
        wait(&status); // Wait for the child process to terminate
        
        if (WIFEXITED(status)) {
            printf("Parent Process: Child exited with status %d\n", WEXITSTATUS(status));
        }
        
        printf("Parent Process: Exiting.\n");
        exit(0);
    }

    return 0;
}
