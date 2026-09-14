#include <iostream>
#include <climits>
using namespace std;

#define MAX 20

struct Process {
    int id, at, bt, rem;
    int ct, wt, tat;
    bool done;
};

void print_results(Process p[], int n) {
    float total_wt = 0, total_tat = 0;
    cout << "\nProcess\tArrival\tBurst\tWaiting\tTurnaround" << endl;
    for (int i = 0; i < n; i++) {
        total_wt += p[i].wt;
        total_tat += p[i].tat;
        cout << "P" << p[i].id << "\t" << p[i].at << "\t" << p[i].bt
             << "\t" << p[i].wt << "\t" << p[i].tat << endl;
    }
    cout << "\nAverage Waiting Time: " << total_wt / n << endl;
    cout << "Average Turnaround Time: " << total_tat / n << endl;
}

void fcfs(Process p[], int n) {
    // Sort by arrival time
    for (int i = 0; i < n - 1; i++)
        for (int j = 0; j < n - i - 1; j++)
            if (p[j].at > p[j+1].at) { Process t = p[j]; p[j] = p[j+1]; p[j+1] = t; }

    int time = 0;
    cout << "Execution Sequence: |";
    for (int i = 0; i < n; i++) {
        if (time < p[i].at) time = p[i].at;
        int start = time;
        time += p[i].bt;
        p[i].ct = time;
        p[i].tat = p[i].ct - p[i].at;
        p[i].wt = p[i].tat - p[i].bt;
        cout << " P" << p[i].id << "(" << start << "-" << time << ") |";
    }
    cout << endl;
    print_results(p, n);
}

void sjf_non_preemptive(Process p[], int n) {
    int time = 0, completed = 0;
    cout << "Execution Sequence: |";
    while (completed < n) {
        int idx = -1, min_bt = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (p[i].at <= time && !p[i].done) {
                if (p[i].bt < min_bt || (p[i].bt == min_bt && p[i].at < p[idx].at)) {
                    min_bt = p[i].bt;
                    idx = i;
                }
            }
        }
        if (idx != -1) {
            int start = time;
            time += p[idx].bt;
            p[idx].ct = time;
            p[idx].tat = p[idx].ct - p[idx].at;
            p[idx].wt = p[idx].tat - p[idx].bt;
            p[idx].done = true;
            completed++;
            cout << " P" << p[idx].id << "(" << start << "-" << time << ") |";
        } else {
            time++;
        }
    }
    cout << endl;
    print_results(p, n);
}

void sjf_preemptive(Process p[], int n) {
    int time = 0, completed = 0, prev = -1, block_start = 0;
    cout << "Execution Sequence: |";
    while (completed < n) {
        int idx = -1, min_rem = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (p[i].at <= time && !p[i].done) {
                if (p[i].rem < min_rem || (p[i].rem == min_rem && p[i].at < p[idx].at)) {
                    min_rem = p[i].rem;
                    idx = i;
                }
            }
        }
        if (idx != -1) {
            if (prev != idx) {
                if (prev != -1)
                    cout << " P" << p[prev].id << "(" << block_start << "-" << time << ") |";
                block_start = time;
                prev = idx;
            }
            p[idx].rem--;
            time++;
            if (p[idx].rem == 0) {
                p[idx].ct = time;
                p[idx].tat = p[idx].ct - p[idx].at;
                p[idx].wt = p[idx].tat - p[idx].bt;
                p[idx].done = true;
                completed++;
                cout << " P" << p[idx].id << "(" << block_start << "-" << time << ") |";
                prev = -1;
            }
        } else {
            if (prev != -1) {
                cout << " P" << p[prev].id << "(" << block_start << "-" << time << ") |";
                prev = -1;
            }
            time++;
        }
    }
    cout << endl;
    print_results(p, n);
}

int main() {
    int n, choice;
    Process p[MAX], backup[MAX];

    cout << "Enter the number of processes (max " << MAX << "): ";
    cin >> n;
    if (n <= 0 || n > MAX) { cout << "Invalid number." << endl; return 1; }

    for (int i = 0; i < n; i++) {
        p[i].id = i + 1;
        cout << "Enter arrival time and burst time for P" << i+1 << ": ";
        cin >> p[i].at >> p[i].bt;
        p[i].rem = p[i].bt;
        p[i].done = false;
        backup[i] = p[i];
    }

    do {
        // Reset
        for (int i = 0; i < n; i++) p[i] = backup[i];

        cout << "\nSelect Scheduling Algorithm:" << endl;
        cout << "1) FCFS" << endl;
        cout << "2) SJF Non-Preemptive" << endl;
        cout << "3) SJF Preemptive (SRTF)" << endl;
        cout << "4) Exit" << endl;
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: cout << "\n--- FCFS Scheduling ---" << endl; fcfs(p, n); break;
            case 2: cout << "\n--- SJF Non-Preemptive ---" << endl; sjf_non_preemptive(p, n); break;
            case 3: cout << "\n--- SJF Preemptive (SRTF) ---" << endl; sjf_preemptive(p, n); break;
            case 4: cout << "Exiting..." << endl; break;
            default: cout << "Invalid choice." << endl;
        }
    } while (choice != 4);

    return 0;
}
