#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <stdbool.h>

#define BUFFER_SIZE 5
#define PRODUCER_ITEMS 10

typedef struct {
    int buffer[BUFFER_SIZE];
    int in, out, count;
    volatile bool flag[2];
    volatile int turn;
} shared_data_t;

void peterson_lock(shared_data_t *shared, int pid) {
    int other = 1 - pid;
    shared->flag[pid] = true;
    shared->turn = other;
    while (shared->flag[other] && shared->turn == other) {
        // busy wait
    }
}

void peterson_unlock(shared_data_t *shared, int pid) {
    shared->flag[pid] = false;
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
    shared->flag[0] = false;
    shared->flag[1] = false;
    shared->turn = 0;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid > 0) {
        // Producer (Process 0)
        for (int i = 1; i <= PRODUCER_ITEMS; i++) {
            while (true) {
                peterson_lock(shared, 0);
                if (shared->count < BUFFER_SIZE) {
                    shared->buffer[shared->in] = i;
                    shared->in = (shared->in + 1) % BUFFER_SIZE;
                    shared->count++;
                    printf("Producer produced: %d\n", i);
                    peterson_unlock(shared, 0);
                    break;
                }
                peterson_unlock(shared, 0);
            }
            usleep(100000);
        }
        wait(NULL);
    } else {
        // Consumer (Process 1)
        for (int i = 1; i <= PRODUCER_ITEMS; i++) {
            while (true) {
                peterson_lock(shared, 1);
                if (shared->count > 0) {
                    int item = shared->buffer[shared->out];
                    shared->out = (shared->out + 1) % BUFFER_SIZE;
                    shared->count--;
                    printf("Consumer consumed: %d\n", item);
                    peterson_unlock(shared, 1);
                    break;
                }
                peterson_unlock(shared, 1);
            }
            usleep(150000);
        }
        exit(0);
    }
    return 0;
}
