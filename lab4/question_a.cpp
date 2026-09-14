#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
using namespace std;

#define MAX_PROC 20
#define MAX_BURST 100
#define MAX_TIME 1000

struct Process {
    int id;
    int priority;
    int arrival;
    int bursts[MAX_BURST]; // alternating CPU, IO, CPU, IO, ...
    int num_bursts;

    int curr_burst;  // index into bursts[]
    int rem_time;    // remaining time for current burst
    int status;      // 0=not arrived, 1=ready(W), 2=running CPU(C), 3=doing IO(R), 4=done
    char history[MAX_TIME + 1];
    int completion_time;
};

Process procs[MAX_PROC], orig[MAX_PROC];
int num_procs;
int quantum = 1;

void read_data() {
    ifstream fin("proc.dat");
    if (!fin) {
        cout << "Error: Cannot open proc.dat" << endl;
        exit(1);
    }

    fin >> num_procs;
    fin.ignore();

    for (int i = 0; i < num_procs; i++) {
        string line;
        getline(fin, line);
        stringstream ss(line);
        char comma;
        ss >> procs[i].id >> comma >> procs[i].priority >> comma >> procs[i].arrival >> comma;
        procs[i].num_bursts = 0;
        int val;
        while (ss >> val) {
            if (val == -1) break;
            procs[i].bursts[procs[i].num_bursts++] = val;
            ss >> comma; // eat comma
        }
    }

    if (!(fin >> quantum)) {
        quantum = 1;
    }
    fin.close();

    for (int i = 0; i < num_procs; i++)
        orig[i] = procs[i];
}

void reset_procs() {
    for (int i = 0; i < num_procs; i++) {
        procs[i] = orig[i];
        procs[i].curr_burst = 0;
        procs[i].rem_time = (procs[i].num_bursts > 0) ? procs[i].bursts[0] : 0;
        procs[i].status = 0;
        memset(procs[i].history, ' ', MAX_TIME);
        procs[i].history[MAX_TIME] = '\0';
        procs[i].completion_time = 0;
    }
}

void print_stats(int total_time) {
    cout << "\n--- Timeline (C=CPU, W=Waiting, R=IO) ---" << endl;
    // Print time header
    cout << "    ";
    for (int t = 0; t < total_time; t++)
        cout << (t % 10);
    cout << endl;

    for (int i = 0; i < num_procs; i++) {
        cout << "P" << procs[i].id << ": ";
        for (int t = 0; t < total_time; t++) {
            char c = procs[i].history[t];
            if (c == ' ') c = '-';
            cout << c;
        }
        cout << endl;
    }

    cout << "\n--- Statistics ---" << endl;
    float avg_tat = 0, avg_wt = 0;
    cout << "PID\tCT\tTAT\tWT" << endl;
    for (int i = 0; i < num_procs; i++) {
        int tat = procs[i].completion_time - procs[i].arrival;
        int total_burst = 0;
        for (int b = 0; b < procs[i].num_bursts; b++)
            total_burst += procs[i].bursts[b];
        int wt = tat - total_burst;
        avg_tat += tat;
        avg_wt += wt;
        cout << procs[i].id << "\t" << procs[i].completion_time
             << "\t" << tat << "\t" << wt << endl;
    }
    cout << "Avg TAT: " << (avg_tat / num_procs)
         << ", Avg WT: " << (avg_wt / num_procs) << endl;
}

void simulate(int algo) {
    reset_procs();
    int time = 0, done = 0, curr_cpu = -1, rr_q = 0;

    int rr_queue[MAX_PROC * 100];
    int rr_head = 0, rr_tail = 0;

    while (done < num_procs && time < MAX_TIME) {
        // 1. Handle arrivals and IO progress
        for (int i = 0; i < num_procs; i++) {
            if (procs[i].status == 0 && procs[i].arrival == time) {
                procs[i].status = 1; // ready
                if (algo == 4) rr_queue[rr_tail++] = i;
            }
            if (procs[i].status == 3) { // doing IO
                procs[i].rem_time--;
                if (procs[i].rem_time == 0) {
                    procs[i].curr_burst++;
                    if (procs[i].curr_burst < procs[i].num_bursts) {
                        procs[i].status = 1;
                        procs[i].rem_time = procs[i].bursts[procs[i].curr_burst];
                        if (algo == 4) rr_queue[rr_tail++] = i;
                    } else {
                        procs[i].status = 4;
                        procs[i].completion_time = time;
                        done++;
                    }
                }
            }
        }

        // 2. CPU scheduling
        if (curr_cpu != -1) {
            procs[curr_cpu].rem_time--;
            rr_q++;

            if (procs[curr_cpu].rem_time == 0) {
                procs[curr_cpu].curr_burst++;
                if (procs[curr_cpu].curr_burst < procs[curr_cpu].num_bursts) {
                    procs[curr_cpu].status = 3; // go to IO
                    procs[curr_cpu].rem_time = procs[curr_cpu].bursts[procs[curr_cpu].curr_burst];
                } else {
                    procs[curr_cpu].status = 4; // done
                    procs[curr_cpu].completion_time = time + 1;
                    done++;
                }
                curr_cpu = -1;
            } else if (algo == 2 || algo == 3) {
                curr_cpu = -1; // force reschedule for preemptive
            } else if (algo == 4 && rr_q == quantum) {
                procs[curr_cpu].status = 1;
                rr_queue[rr_tail++] = curr_cpu;
                curr_cpu = -1;
            }
        }

        if (curr_cpu == -1) {
            if (algo == 4) { // Round Robin
                if (rr_head < rr_tail) {
                    curr_cpu = rr_queue[rr_head++];
                    procs[curr_cpu].status = 2;
                    rr_q = 0;
                }
            } else { // FCFS, SJF, Priority
                int best = -1;
                for (int i = 0; i < num_procs; i++) {
                    if (procs[i].status == 1 || procs[i].status == 2) {
                        if (best == -1) {
                            best = i;
                        } else if (algo == 2) { // SJF
                            if (procs[i].rem_time < procs[best].rem_time) best = i;
                        } else if (algo == 3) { // Priority
                            if (procs[i].priority < procs[best].priority) best = i;
                        }
                        // FCFS: first found in order is fine
                    }
                }
                if (best != -1) {
                    if (curr_cpu != -1 && curr_cpu != best && procs[curr_cpu].status == 2)
                        procs[curr_cpu].status = 1;
                    curr_cpu = best;
                    procs[curr_cpu].status = 2;
                }
            }
        }

        // 3. Record history
        for (int i = 0; i < num_procs; i++) {
            if (procs[i].status == 1)      procs[i].history[time] = 'W';
            else if (procs[i].status == 2)  procs[i].history[time] = 'C';
            else if (procs[i].status == 3)  procs[i].history[time] = 'R';
        }

        time++;
    }

    print_stats(time);
}

int main() {
    read_data();

    cout << "Select Scheduling Algorithm:" << endl;
    cout << "1. FCFS" << endl;
    cout << "2. SJF (Preemptive)" << endl;
    cout << "3. Priority (Preemptive)" << endl;
    cout << "4. Round Robin (quantum=" << quantum << ")" << endl;
    cout << "Enter choice (1-4): ";

    int choice;
    cin >> choice;
    if (choice < 1 || choice > 4) {
        cout << "Invalid choice." << endl;
        return 1;
    }

    simulate(choice);
    return 0;
}
