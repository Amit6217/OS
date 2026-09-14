#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>

int main() {
    // Create shared memory
    int *VAR = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (VAR == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    
    *VAR = 10;
    printf("Initial VAR = %d\n", *VAR);

    // Unlink old semaphore if exists
    sem_unlink("/lab6_sem");

    // Create named semaphore initialized to 0
    sem_t *sem = sem_open("/lab6_sem", O_CREAT, 0666, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        *VAR -= 2;
        printf("Child: VAR = %d (after decrement by 2)\n", *VAR);
        sem_post(sem); // Signal parent
        exit(0);
    } else {
        // Parent process
        sem_wait(sem); // Wait for child
        *VAR += 2;
        printf("Parent: VAR = %d (after increment by 2)\n", *VAR);
        wait(NULL); // Reap child process
    }

    // Clean up
    sem_close(sem);
    sem_unlink("/lab6_sem");
    munmap(VAR, sizeof(int));

    return 0;
}
