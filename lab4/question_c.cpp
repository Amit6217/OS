#include <iostream>
#include <climits>
using namespace std;

#define MAX 20

struct Process {
    int id, at, bt, pr;
    int rem, ct, wt, tat;
    bool done;
};

struct Gantt {
    int pid, start, end;
} gantt[1000];
int g_len = 0;

void add_gantt(int pid, int s, int e) {
    if (g_len > 0 && gantt[g_len-1].pid == pid && gantt[g_len-1].end == s)
        gantt[g_len-1].end = e;
    else {
        gantt[g_len].pid = pid;
        gantt[g_len].start = s;
        gantt[g_len].end = e;
        g_len++;
    }
}

void print_gantt() {
    cout << "\nExecution Sequence: | ";
    for (int i = 0; i < g_len; i++) {
        if (gantt[i].pid == -1)
            cout << "IDLE(" << gantt[i].start << "-" << gantt[i].end << ") | ";
        else
            cout << "P" << gantt[i].pid << "(" << gantt[i].start << "-" << gantt[i].end << ") | ";
    }
    cout << endl;
}

void print_results(Process p[], int n) {
    float total_wt = 0, total_tat = 0;
    cout << "\nPID\tArrival\tBurst\tPriority\tWaiting\tTurnaround" << endl;
    for (int i = 0; i < n; i++) {
        total_wt += p[i].wt;
        total_tat += p[i].tat;
        cout << "P" << p[i].id << "\t" << p[i].at << "\t" << p[i].bt
             << "\t" << p[i].pr << "\t\t" << p[i].wt << "\t" << p[i].tat << endl;
    }
    cout << "\nAverage Waiting Time: " << total_wt / n << endl;
    cout << "Average Turnaround Time: " << total_tat / n << endl;
}

void reset(Process p[], int n) {
    for (int i = 0; i < n; i++) {
        p[i].rem = p[i].bt;
        p[i].done = false;
        p[i].wt = p[i].tat = p[i].ct = 0;
    }
    g_len = 0;
}

void priority_non_preemptive(Process p[], int n) {
    reset(p, n);
    int time = 0, completed = 0;

    while (completed < n) {
        int idx = -1, best_pr = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (p[i].at <= time && !p[i].done) {
                if (p[i].pr < best_pr || (p[i].pr == best_pr && (idx == -1 || p[i].at < p[idx].at))) {
                    best_pr = p[i].pr;
                    idx = i;
                }
            }
        }
        if (idx != -1) {
            add_gantt(p[idx].id, time, time + p[idx].bt);
            time += p[idx].bt;
            p[idx].done = true;
            p[idx].ct = time;
            p[idx].tat = p[idx].ct - p[idx].at;
            p[idx].wt = p[idx].tat - p[idx].bt;
            completed++;
        } else {
            add_gantt(-1, time, time + 1);
            time++;
        }
    }
    print_gantt();
    print_results(p, n);
}

void priority_preemptive(Process p[], int n) {
    reset(p, n);
    int time = 0, completed = 0;

    while (completed < n) {
        int idx = -1, best_pr = INT_MAX;
        for (int i = 0; i < n; i++) {
            if (p[i].at <= time && !p[i].done) {
                if (p[i].pr < best_pr || (p[i].pr == best_pr && (idx == -1 || p[i].at < p[idx].at))) {
                    best_pr = p[i].pr;
                    idx = i;
                }
            }
        }
        if (idx != -1) {
            add_gantt(p[idx].id, time, time + 1);
            p[idx].rem--;
            time++;
            if (p[idx].rem == 0) {
                p[idx].done = true;
                p[idx].ct = time;
                p[idx].tat = p[idx].ct - p[idx].at;
                p[idx].wt = p[idx].tat - p[idx].bt;
                completed++;
            }
        } else {
            add_gantt(-1, time, time + 1);
            time++;
        }
    }
    print_gantt();
    print_results(p, n);
}

void round_robin(Process p[], int n, int quantum) {
    reset(p, n);
    int time = 0, completed = 0;
    int queue[1000], front = 0, rear = 0;
    bool in_q[MAX] = {false};

    // Add processes arriving at time 0
    for (int i = 0; i < n; i++) {
        if (p[i].at <= 0) {
            queue[rear++] = i;
            in_q[i] = true;
        }
    }

    while (completed < n) {
        if (front == rear) { // queue empty, jump to next arrival
            int next = INT_MAX;
            for (int i = 0; i < n; i++)
                if (!p[i].done && p[i].at > time && p[i].at < next)
                    next = p[i].at;
            if (next != INT_MAX) {
                add_gantt(-1, time, next);
                time = next;
                for (int i = 0; i < n; i++)
                    if (p[i].at <= time && !in_q[i] && !p[i].done) {
                        queue[rear++] = i;
                        in_q[i] = true;
                    }
            }
            continue;
        }

        int idx = queue[front++];
        int run = (p[idx].rem > quantum) ? quantum : p[idx].rem;

        add_gantt(p[idx].id, time, time + run);
        time += run;
        p[idx].rem -= run;

        // Add newly arrived processes
        for (int i = 0; i < n; i++)
            if (p[i].at <= time && !in_q[i] && !p[i].done) {
                queue[rear++] = i;
                in_q[i] = true;
            }

        if (p[idx].rem == 0) {
            p[idx].done = true;
            p[idx].ct = time;
            p[idx].tat = p[idx].ct - p[idx].at;
            p[idx].wt = p[idx].tat - p[idx].bt;
            completed++;
        } else {
            queue[rear++] = idx;
        }
    }
    print_gantt();
    print_results(p, n);
}

int main() {
    int n;
    Process p[MAX];

    cout << "Enter number of processes: ";
    cin >> n;
    if (n <= 0 || n > MAX) return 1;

    for (int i = 0; i < n; i++) {
        p[i].id = i + 1;
        cout << "P" << i+1 << " - Enter Arrival Time, Burst Time, Priority: ";
        cin >> p[i].at >> p[i].bt >> p[i].pr;
    }

    int choice;
    do {
        cout << "\nSelect Scheduling Algorithm:" << endl;
        cout << "1. Priority Non-Preemptive" << endl;
        cout << "2. Priority Preemptive" << endl;
        cout << "3. Round Robin" << endl;
        cout << "4. Exit" << endl;
        cout << "Choice: ";
        cin >> choice;

        switch (choice) {
            case 1: cout << "\n--- Priority Non-Preemptive ---" << endl; priority_non_preemptive(p, n); break;
            case 2: cout << "\n--- Priority Preemptive ---" << endl; priority_preemptive(p, n); break;
            case 3: {
                int q;
                cout << "Enter Time Quantum: ";
                cin >> q;
                if (q > 0) { cout << "\n--- Round Robin ---" << endl; round_robin(p, n, q); }
                else cout << "Invalid quantum!" << endl;
                break;
            }
            case 4: cout << "Exiting..." << endl; break;
            default: cout << "Invalid choice!" << endl;
        }
    } while (choice != 4);

    return 0;
}
