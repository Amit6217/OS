#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdbool.h>

#define BUFFER_SIZE 5
#define PRODUCER_ITEMS 10
#define N 2 // Two processes

typedef struct {
    int buffer[BUFFER_SIZE];
    int in, out, count;
    volatile bool choosing[N];
    volatile int number[N];
} shared_data_t;

int max_number(shared_data_t *shared) {
    int max = 0;
    for (int i = 0; i < N; i++) {
        if (shared->number[i] > max) {
            max = shared->number[i];
        }
    }
    return max;
}

void lamport_lock(shared_data_t *shared, int pid) {
    shared->choosing[pid] = true;
    shared->number[pid] = 1 + max_number(shared);
    shared->choosing[pid] = false;

    for (int j = 0; j < N; j++) {
        if (j == pid) continue;
        while (shared->choosing[j]) {
            // busy wait
        }
        while (shared->number[j] != 0 &&
               (shared->number[j] < shared->number[pid] ||
                (shared->number[j] == shared->number[pid] && j < pid))) {
            // busy wait
        }
    }
}

void lamport_unlock(shared_data_t *shared, int pid) {
    shared->number[pid] = 0;
}

int main() {
    shared_data_t *shared = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (shared == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }
    
    shared->in = 0;
    shared->out = 0;
    shared->count = 0;
    for (int i = 0; i < N; i++) {
        shared->choosing[i] = false;
        shared->number[i] = 0;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid > 0) {
        // Producer (Process 0)
        for (int i = 1; i <= PRODUCER_ITEMS; i++) {
            while (true) {
                lamport_lock(shared, 0);
                if (shared->count < BUFFER_SIZE) {
                    shared->buffer[shared->in] = i;
                    shared->in = (shared->in + 1) % BUFFER_SIZE;
                    shared->count++;
                    printf("Producer produced: %d\n", i);
                    lamport_unlock(shared, 0);
                    break;
                }
                lamport_unlock(shared, 0);
            }
            usleep(100000);
        }
        wait(NULL);
    } else {
        // Consumer (Process 1)
        for (int i = 1; i <= PRODUCER_ITEMS; i++) {
            while (true) {
                lamport_lock(shared, 1);
                if (shared->count > 0) {
                    int item = shared->buffer[shared->out];
                    shared->out = (shared->out + 1) % BUFFER_SIZE;
                    shared->count--;
                    printf("Consumer consumed: %d\n", item);
                    lamport_unlock(shared, 1);
                    break;
                }
                lamport_unlock(shared, 1);
            }
            usleep(150000);
        }
        exit(0);
    }
    return 0;
}
