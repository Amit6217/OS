#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int global = 10;

int main() {
    printf("Before fork:\n");
    printf("Parent: value = %d, address = %p\n", global, (void*)&global);

    pid_t pid = fork();

    if (pid == 0) {
        // Child
        printf("\nChild BEFORE modification:\n");
        printf("Child:  value = %d, address = %p\n", global, (void*)&global);

        // Modify the variable → Copy-on-Write happens
        global = 20;

        printf("\nChild AFTER modification:\n");
        printf("Child:  value = %d, address = %p\n", global, (void*)&global);
    }
    else {
        // Parent waits so output is easy to understand
        wait(NULL);

        printf("\nParent AFTER child modification:\n");
        printf("Parent: value = %d, address = %p\n", global, (void*)&global);
    }

    return 0;
}