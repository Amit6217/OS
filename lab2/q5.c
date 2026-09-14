#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int global_var = 10; // Global variable

int main() {
    static int static_var = 20; // Static variable
    int local_var = 30;         // Local variable

    printf("--- Before fork ---\n");
    printf("Global: %d, Static: %d, Local: %d\n", global_var, static_var, local_var);
    printf("Addresses - Global: %p, Static: %p, Local: %p\n\n", 
           (void*)&global_var, (void*)&static_var, (void*)&local_var);

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process
        printf("--- Child Process (PID: %d) ---\n", getpid());
        
        // Modify variables
        global_var += 5;
        static_var += 5;
        local_var += 5;
        
        printf("Values    - Global: %d, Static: %d, Local: %d\n", global_var, static_var, local_var);
        printf("Addresses - Global: %p, Static: %p, Local: %p\n\n", 
               (void*)&global_var, (void*)&static_var, (void*)&local_var);
        
        exit(0);
    } else {
        // Parent process
        wait(NULL); // Wait for child to finish
        printf("--- Parent Process (PID: %d) ---\n", getpid());
        
        printf("Values    - Global: %d, Static: %d, Local: %d\n", global_var, static_var, local_var);
        printf("Addresses - Global: %p, Static: %p, Local: %p\n", 
               (void*)&global_var, (void*)&static_var, (void*)&local_var);
    }

    return 0;
}
