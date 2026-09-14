#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUM_ACCOUNTS 3
#define NUM_CLIENTS 5
#define NUM_TRANSACTIONS 5

typedef struct {
    int balance;
    pthread_mutex_t lock;
} Account;

Account accounts[NUM_ACCOUNTS];

void* client_thread(void* arg) {
    int client_id = *(int*)arg;
    for (int i = 0; i < NUM_TRANSACTIONS; i++) {
        int account_idx = rand() % NUM_ACCOUNTS;
        int amount = (rand() % 500) + 1;
        int is_deposit = rand() % 2;

        pthread_mutex_lock(&accounts[account_idx].lock);
        
        if (is_deposit) {
            accounts[account_idx].balance += amount;
            printf("Client %d: Deposited $%d to Account %d (Balance: $%d)\n", client_id, amount, account_idx, accounts[account_idx].balance);
        } else {
            if (accounts[account_idx].balance >= amount) {
                accounts[account_idx].balance -= amount;
                printf("Client %d: Withdrew $%d from Account %d (Balance: $%d)\n", client_id, amount, account_idx, accounts[account_idx].balance);
            } else {
                printf("Client %d: Withdrawal of $%d from Account %d failed (Insufficient balance: $%d)\n", client_id, amount, account_idx, accounts[account_idx].balance);
            }
        }
        
        pthread_mutex_unlock(&accounts[account_idx].lock);
        usleep((rand() % 100) * 1000); // sleep a bit to allow interleaving
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_t clients[NUM_CLIENTS];
    int client_ids[NUM_CLIENTS];

    // Initialize accounts
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        accounts[i].balance = 1000;
        pthread_mutex_init(&accounts[i].lock, NULL);
    }

    // Create client threads
    for (int i = 0; i < NUM_CLIENTS; i++) {
        client_ids[i] = i + 1;
        pthread_create(&clients[i], NULL, client_thread, &client_ids[i]);
    }

    // Join client threads
    for (int i = 0; i < NUM_CLIENTS; i++) {
        pthread_join(clients[i], NULL);
    }

    // Print final balances
    printf("\nFinal Balances:\n");
    for (int i = 0; i < NUM_ACCOUNTS; i++) {
        printf("Account %d: $%d\n", i, accounts[i].balance);
        pthread_mutex_destroy(&accounts[i].lock);
    }

    return 0;
}
