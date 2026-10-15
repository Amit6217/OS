#include <iostream>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>

using namespace std;

struct SharedData {
    sem_t mutex;
    sem_t empty;
    sem_t full;
    int in;
    int out;
    int buffer[100];
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <buffer_size>" << endl;
        return 1;
    }

    int buffer_size = atoi(argv[1]);
    int total_items = 10;

    // Shared memory for independent processes
    SharedData* shm = (SharedData*)mmap(NULL, sizeof(SharedData), 
                                        PROT_READ | PROT_WRITE, 
                                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    shm->in = 0;
    shm->out = 0;

    // pshared = 1 for process-shared semaphores
    sem_init(&shm->mutex, 1, 1);
    sem_init(&shm->empty, 1, buffer_size);
    sem_init(&shm->full, 1, 0);

    pid_t pid = fork();

    if (pid == 0) {
        // Consumer process
        for (int i = 0; i < total_items; i++) {
            sem_wait(&shm->full);
            sem_wait(&shm->mutex);

            int item = shm->buffer[shm->out];
            shm->out = (shm->out + 1) % buffer_size;
            cout << "[Consumer Process] Consumed item " << item << endl;

            sem_post(&shm->mutex);
            sem_post(&shm->empty);

            usleep(80000);
        }
        return 0;
    } else {
        // Producer process
        for (int item = 1; item <= total_items; item++) {
            sem_wait(&shm->empty);
            sem_wait(&shm->mutex);

            shm->buffer[shm->in] = item;
            shm->in = (shm->in + 1) % buffer_size;
            cout << "[Producer Process] Produced item " << item << endl;

            sem_post(&shm->mutex);
            sem_post(&shm->full);

            usleep(50000);
        }

        // Wait for consumer process to finish
        wait(NULL);

        sem_destroy(&shm->mutex);
        sem_destroy(&shm->empty);
        sem_destroy(&shm->full);
        munmap(shm, sizeof(SharedData));
    }

    return 0;
}
