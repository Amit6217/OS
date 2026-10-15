#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

struct Item {
    int p_id;
    int x;
};

Item *buffer;
int buffer_size;
int in = 0, out = 0;
int items_per_prod = 4;

sem_t mutex_sem, empty_sem, full_sem;

void* producer(void* arg) {
    int p_id = *(int*)arg;

    for (int x = 1; x <= items_per_prod; x++) {
        sem_wait(&empty_sem);
        sem_wait(&mutex_sem);

        buffer[in] = {p_id, x};
        in = (in + 1) % buffer_size;
        cout << "[Producer " << p_id << "] Produced item " << p_id << "." << x << endl;

        sem_post(&mutex_sem);
        sem_post(&full_sem);

        usleep(40000);
    }
    return NULL;
}

void* consumer(void* arg) {
    int c_id = *(int*)arg;

    while (true) {
        sem_wait(&full_sem);
        sem_wait(&mutex_sem);

        Item item = buffer[out];
        out = (out + 1) % buffer_size;

        sem_post(&mutex_sem);
        sem_post(&empty_sem);

        // Sentinel value -1 means stop
        if (item.p_id == -1) {
            break;
        }

        cout << "[Consumer " << c_id << "] Consumed item " << item.p_id << "." << item.x << endl;

        usleep(60000);
    }
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cout << "Usage: " << argv[0] << " <buffer_size> <num_producers> <num_consumers>" << endl;
        return 1;
    }

    buffer_size = atoi(argv[1]);
    int num_producers = atoi(argv[2]);
    int num_consumers = atoi(argv[3]);

    buffer = new Item[buffer_size];

    sem_init(&mutex_sem, 0, 1);
    sem_init(&empty_sem, 0, buffer_size);
    sem_init(&full_sem, 0, 0);

    pthread_t prods[num_producers];
    pthread_t conss[num_consumers];
    int prod_ids[num_producers];
    int cons_ids[num_consumers];

    for (int i = 0; i < num_producers; i++) {
        prod_ids[i] = i + 1;
        pthread_create(&prods[i], NULL, producer, &prod_ids[i]);
    }

    for (int i = 0; i < num_consumers; i++) {
        cons_ids[i] = i + 1;
        pthread_create(&conss[i], NULL, consumer, &cons_ids[i]);
    }

    // Wait for producers to finish
    for (int i = 0; i < num_producers; i++) {
        pthread_join(prods[i], NULL);
    }

    // Send stop signal (-1) to consumers
    for (int i = 0; i < num_consumers; i++) {
        sem_wait(&empty_sem);
        sem_wait(&mutex_sem);
        buffer[in] = {-1, -1};
        in = (in + 1) % buffer_size;
        sem_post(&mutex_sem);
        sem_post(&full_sem);
    }

    // Wait for consumers to finish
    for (int i = 0; i < num_consumers; i++) {
        pthread_join(conss[i], NULL);
    }

    sem_destroy(&mutex_sem);
    sem_destroy(&empty_sem);
    sem_destroy(&full_sem);
    delete[] buffer;

    return 0;
}
