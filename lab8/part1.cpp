#include <iostream>
#include <vector>

using namespace std;

const int P = 4; // Number of processes
const int R = 4; // Number of resource classes (A, B, C, D)

// Check if system is in a safe state and find safe sequence
bool isSafe(int available[], int allocation[][R], int need[][R], vector<int>& safeSeq) {
    int work[R];
    for (int i = 0; i < R; i++) work[i] = available[i];

    bool finish[P] = {false};
    safeSeq.clear();

    for (int count = 0; count < P; count++) {
        bool found = false;
        for (int p = 0; p < P; p++) {
            if (!finish[p]) {
                bool canAllocate = true;
                for (int j = 0; j < R; j++) {
                    if (need[p][j] > work[j]) {
                        canAllocate = false;
                        break;
                    }
                }

                if (canAllocate) {
                    for (int j = 0; j < R; j++) {
                        work[j] += allocation[p][j];
                    }
                    finish[p] = true;
                    safeSeq.push_back(p);
                    found = true;
                    break;
                }
            }
        }
        if (!found) return false; // Unsafe state
    }
    return true;
}

// Resource-Request algorithm simulation
void testRequest(int p_id, int request[], int avail_orig[], int alloc_orig[][R], int need_orig[][R], const string& title) {
    // Copy state to test request independently
    int avail[R];
    int alloc[P][R];
    int need[P][R];

    for (int j = 0; j < R; j++) avail[j] = avail_orig[j];
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < R; j++) {
            alloc[i][j] = alloc_orig[i][j];
            need[i][j] = need_orig[i][j];
        }
    }

    cout << "\n==================================================\n";
    cout << title << "\n";
    cout << "Process P" << p_id << " requests: [ ";
    for (int j = 0; j < R; j++) cout << request[j] << " ";
    cout << "]\n";

    // 1. Check if Request <= Need
    for (int j = 0; j < R; j++) {
        if (request[j] > need[p_id][j]) {
            cout << "Error: Process exceeded its maximum claim.\n";
            return;
        }
    }

    // 2. Check if Request <= Available
    for (int j = 0; j < R; j++) {
        if (request[j] > avail[j]) {
            cout << "Resources not currently available. Process must wait.\n";
            return;
        }
    }

    // 3. Pretend to allocate
    for (int j = 0; j < R; j++) {
        avail[j] -= request[j];
        alloc[p_id][j] += request[j];
        need[p_id][j] -= request[j];
    }

    // 4. Test safety
    vector<int> safeSeq;
    if (isSafe(avail, alloc, need, safeSeq)) {
        cout << "Status: Request GRANTED immediately.\n";
        cout << "Safe Sequence exists: ";
        for (size_t i = 0; i < safeSeq.size(); i++) {
            cout << "P" << safeSeq[i] << (i + 1 < safeSeq.size() ? " -> " : "\n");
        }
    } else {
        cout << "Status: Request DENIED (Deadlock Avoidance).\n";
        cout << "Reason: Allocation leads to an UNSAFE state (No safe sequence exists).\n";
        cout << "Process P" << p_id << " must wait to prevent deadlock.\n";
    }
}

int main() {
    int available[R] = {1, 1, 2, 1};

    int allocation[P][R] = {
        {1, 0, 2, 1}, // P0
        {2, 0, 1, 1}, // P1
        {1, 1, 0, 2}, // P2
        {1, 1, 1, 0}  // P3
    };

    int max[P][R] = {
        {2, 2, 3, 2}, // P0
        {3, 1, 2, 2}, // P1
        {3, 2, 2, 3}, // P2
        {2, 3, 2, 2}  // P3
    };

    int need[P][R];
    for (int i = 0; i < P; i++) {
        for (int j = 0; j < R; j++) {
            need[i][j] = max[i][j] - allocation[i][j];
        }
    }

    cout << "--- OS Lab 8: Banker's Algorithm Simulation ---\n";
    cout << "Processes: P0, P1, P2, P3 | Resources: A, B, C, D\n\n";

    cout << "Initial Available: [ ";
    for (int j = 0; j < R; j++) cout << available[j] << " ";
    cout << "]\n\n";

    cout << "PID\tAllocated\tMax\t\tNeed\n";
    for (int i = 0; i < P; i++) {
        cout << "P" << i << "\t";
        for (int j = 0; j < R; j++) cout << allocation[i][j] << " ";
        cout << "\t";
        for (int j = 0; j < R; j++) cout << max[i][j] << " ";
        cout << "\t";
        for (int j = 0; j < R; j++) cout << need[i][j] << " ";
        cout << "\n";
    }

    vector<int> initialSafeSeq;
    if (isSafe(available, allocation, need, initialSafeSeq)) {
        cout << "\nInitial System State: SAFE\nSafe Sequence: ";
        for (size_t i = 0; i < initialSafeSeq.size(); i++) {
            cout << "P" << initialSafeSeq[i] << (i + 1 < initialSafeSeq.size() ? " -> " : "\n");
        }
    }

    // Case 1: Safe request (Safe sequence exists)
    int req1[R] = {1, 0, 1, 0};
    testRequest(1, req1, available, allocation, need, "Case 1: Safe Request (Safe Sequence Exists)");

    // Case 2: Unsafe request (No safe sequence exists)
    int req2[R] = {1, 0, 1, 0};
    testRequest(2, req2, available, allocation, need, "Case 2: Unsafe Request (No Safe Sequence Exists)");

    return 0;
}
