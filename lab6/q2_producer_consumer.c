#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 5
#define MAX_THREADS 10
#define ITEMS_TO_PRODUCE 10

int buffer[BUFFER_SIZE];
int in = 0, out = 0, count = 0;

// Variables to handle termination for multiple consumers
int total_items_to_consume = 0;
int items_consumed = 0;

// Peterson's variables for 2 threads
volatile int flag[2] = {0, 0};
volatile int turn = 0;

// Lamport's Bakery variables for N threads
volatile int choosing[MAX_THREADS] = {0};
volatile int ticket[MAX_THREADS] = {0};
int num_competing_threads = 0;

void lock(int id) {
    int other = 1 - id;
    flag[id] = 1;
    turn = other;
    while (flag[other] && turn == other); // busy wait
}

void unlock(int id) {
    flag[id] = 0;
}

void bakery_lock(int id, int n) {
    choosing[id] = 1;
    int max_ticket = 0;
    for (int j = 0; j < n; j++)
        if (ticket[j] > max_ticket) max_ticket = ticket[j];
    ticket[id] = max_ticket + 1;
    choosing[id] = 0;
    for (int j = 0; j < n; j++) {
        while (choosing[j]); // busy wait
        while (ticket[j] != 0 && (ticket[j] < ticket[id] || (ticket[j] == ticket[id] && j < id))); // busy wait
    }
}

void bakery_unlock(int id) {
    ticket[id] = 0;
}

// Thread functions for Variant 1 (Peterson's)
void* producer_v1(void* arg) {
    int id = 0;
    for (int i = 0; i < ITEMS_TO_PRODUCE; i++) {
        int item = rand() % 100;
        
        while (1) {
            lock(id);
            if (count < BUFFER_SIZE) {
                buffer[in] = item;
                in = (in + 1) % BUFFER_SIZE;
                count++;
                printf("Producer %d: Produced item %d (buffer count: %d)\n", id, item, count);
                unlock(id);
                break;
            }
            unlock(id);
        }
        usleep(10000); // Sleep briefly to allow interleaving
    }
    return NULL;
}

void* consumer_v1(void* arg) {
    int id = 1;
    for (int i = 0; i < ITEMS_TO_PRODUCE; i++) {
        while (1) {
            lock(id);
            if (count > 0) {
                int item = buffer[out];
                out = (out + 1) % BUFFER_SIZE;
                count--;
                printf("Consumer %d: Consumed item %d (buffer count: %d)\n", id, item, count);
                unlock(id);
                break;
            }
            unlock(id);
        }
        usleep(15000);
    }
    return NULL;
}

// Thread data and functions for Variants 2, 3, 4 (Lamport's Bakery)
typedef struct {
    int thread_id;
    int items_to_produce;
    int is_producer;
} ThreadData;

void* worker_vn(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int id = data->thread_id;
    
    if (data->is_producer) {
        for (int i = 0; i < data->items_to_produce; i++) {
            int item = rand() % 100;
            while (1) {
                bakery_lock(id, num_competing_threads);
                if (count < BUFFER_SIZE) {
                    buffer[in] = item;
                    in = (in + 1) % BUFFER_SIZE;
                    count++;
                    printf("Producer %d: Produced item %d (buffer count: %d)\n", id, item, count);
                    bakery_unlock(id);
                    break;
                }
                bakery_unlock(id);
            }
            usleep(10000);
        }
    } else {
        while (1) {
            bakery_lock(id, num_competing_threads);
            if (items_consumed >= total_items_to_consume) {
                bakery_unlock(id);
                break; // Done consuming all items produced
            }
            if (count > 0) {
                int item = buffer[out];
                out = (out + 1) % BUFFER_SIZE;
                count--;
                items_consumed++;
                printf("Consumer %d: Consumed item %d (buffer count: %d)\n", id, item, count);
                bakery_unlock(id);
                usleep(15000);
            } else {
                bakery_unlock(id);
            }
        }
    }
    return NULL;
}

void run_variant_1() {
    pthread_t prod, cons;
    in = out = count = 0;
    flag[0] = flag[1] = 0;
    turn = 0;
    
    pthread_create(&prod, NULL, producer_v1, NULL);
    pthread_create(&cons, NULL, consumer_v1, NULL);
    
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
}

void run_variant_n(int num_producers, int num_consumers) {
    pthread_t threads[MAX_THREADS];
    ThreadData tdata[MAX_THREADS];
    
    in = out = count = 0;
    items_consumed = 0;
    total_items_to_consume = num_producers * ITEMS_TO_PRODUCE;
    num_competing_threads = num_producers + num_consumers;
    
    for (int i = 0; i < num_competing_threads; i++) {
        choosing[i] = 0;
        ticket[i] = 0;
    }
    
    int t_idx = 0;
    for (int i = 0; i < num_producers; i++) {
        tdata[t_idx].thread_id = t_idx;
        tdata[t_idx].items_to_produce = ITEMS_TO_PRODUCE;
        tdata[t_idx].is_producer = 1;
        pthread_create(&threads[t_idx], NULL, worker_vn, &tdata[t_idx]);
        t_idx++;
    }
    
    for (int i = 0; i < num_consumers; i++) {
        tdata[t_idx].thread_id = t_idx;
        tdata[t_idx].is_producer = 0;
        pthread_create(&threads[t_idx], NULL, worker_vn, &tdata[t_idx]);
        t_idx++;
    }
    
    for (int i = 0; i < num_competing_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main() {
    srand(time(NULL));
    int choice;
    
    while(1) {
        printf("\nMenu:\n");
        printf("1. Single Producer - Single Consumer (Peterson's)\n");
        printf("2. Single Producer - Multiple Consumers (Lamport's Bakery)\n");
        printf("3. Multiple Producers - Single Consumer (Lamport's Bakery)\n");
        printf("4. Multiple Producers - Multiple Consumers (Lamport's Bakery)\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) break;
        
        switch(choice) {
            case 1:
                printf("\n--- Single Producer, Single Consumer ---\n");
                run_variant_1();
                break;
            case 2:
                printf("\n--- 1 Producer, 3 Consumers ---\n");
                run_variant_n(1, 3);
                break;
            case 3:
                printf("\n--- 3 Producers, 1 Consumer ---\n");
                run_variant_n(3, 1);
                break;
            case 4:
                printf("\n--- 3 Producers, 3 Consumers ---\n");
                run_variant_n(3, 3);
                break;
            case 5:
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid choice.\n");
        }
    }
    return 0;
}
