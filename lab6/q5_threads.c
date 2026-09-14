#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int data[10] = {12, 5, 23, 7, 45, 18, 3, 34, 9, 27};
int sum_result;
int max_result;
int min_result;

void* calc_sum(void* arg) {
    sum_result = 0;
    for (int i = 0; i < 10; i++) {
        sum_result += data[i];
    }
    return NULL;
}

void* calc_max(void* arg) {
    max_result = data[0];
    for (int i = 1; i < 10; i++) {
        if (data[i] > max_result) {
            max_result = data[i];
        }
    }
    return NULL;
}

void* calc_min(void* arg) {
    min_result = data[0];
    for (int i = 1; i < 10; i++) {
        if (data[i] < min_result) {
            min_result = data[i];
        }
    }
    return NULL;
}

int main() {
    pthread_t thread_sum, thread_max, thread_min;

    // Create threads
    pthread_create(&thread_sum, NULL, calc_sum, NULL);
    pthread_create(&thread_max, NULL, calc_max, NULL);
    pthread_create(&thread_min, NULL, calc_min, NULL);

    // Wait for threads to finish
    pthread_join(thread_sum, NULL);
    pthread_join(thread_max, NULL);
    pthread_join(thread_min, NULL);

    // Print results
    printf("Sum = %d\n", sum_result);
    printf("Max = %d\n", max_result);
    printf("Min = %d\n", min_result);

    return 0;
}
