#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

int *buffer;
int buffer_size;
int in = 0, out = 0;
int total_items = 10;

sem_t mutex_sem, empty_sem, full_sem;

void* producer(void* arg) {
    for (int item = 1; item <= total_items; item++) {
        sem_wait(&empty_sem);
        sem_wait(&mutex_sem);

        buffer[in] = item;
        in = (in + 1) % buffer_size;
        cout << "[Producer] Produced item " << item << endl;

        sem_post(&mutex_sem);
        sem_post(&full_sem);

        usleep(50000);
    }
    return NULL;
}

void* consumer(void* arg) {
    for (int i = 0; i < total_items; i++) {
        sem_wait(&full_sem);
        sem_wait(&mutex_sem);

        int item = buffer[out];
        out = (out + 1) % buffer_size;
        cout << "[Consumer] Consumed item " << item << endl;

        sem_post(&mutex_sem);
        sem_post(&empty_sem);

        usleep(80000);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cout << "Usage: " << argv[0] << " <buffer_size>" << endl;
        return 1;
    }

    buffer_size = atoi(argv[1]);
    buffer = new int[buffer_size];

    sem_init(&mutex_sem, 0, 1);
    sem_init(&empty_sem, 0, buffer_size);
    sem_init(&full_sem, 0, 0);

    pthread_t prod, cons;
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);

    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    sem_destroy(&mutex_sem);
    sem_destroy(&empty_sem);
    sem_destroy(&full_sem);
    delete[] buffer;

    return 0;
}
