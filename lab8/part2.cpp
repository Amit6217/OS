#include <iostream>
#include <fstream>
#include <vector>
#include <iomanip>

using namespace std;

struct Request {
    int pid;
    int arrival_time;
    int block_size;
    int duration;
};

struct Partition {
    int start;
    int size;
    bool is_free;
    int pid;
    int end_time;
};

// Release finished allocations and merge adjacent free blocks
void freeCompleted(vector<Partition>& memory, int current_time) {
    for (auto& p : memory) {
        if (!p.is_free && p.end_time <= current_time) {
            p.is_free = true;
            p.pid = -1;
        }
    }

    // Coalesce adjacent free blocks
    for (size_t i = 0; i + 1 < memory.size(); ) {
        if (memory[i].is_free && memory[i + 1].is_free) {
            memory[i].size += memory[i + 1].size;
            memory.erase(memory.begin() + i + 1);
        } else {
            i++;
        }
    }
}

// Find partition index based on strategy: 0=First Fit, 1=Best Fit, 2=Worst Fit
int findPartition(const vector<Partition>& memory, int req_size, int strategy) {
    int chosen_idx = -1;

    for (size_t i = 0; i < memory.size(); i++) {
        if (memory[i].is_free && memory[i].size >= req_size) {
            if (strategy == 0) { // First Fit
                return i;
            } else if (strategy == 1) { // Best Fit
                if (chosen_idx == -1 || memory[i].size < memory[chosen_idx].size) {
                    chosen_idx = i;
                }
            } else if (strategy == 2) { // Worst Fit
                if (chosen_idx == -1 || memory[i].size > memory[chosen_idx].size) {
                    chosen_idx = i;
                }
            }
        }
    }
    return chosen_idx;
}

// Calculate total free memory
int getFreeMemory(const vector<Partition>& memory) {
    int free_mem = 0;
    for (const auto& p : memory) {
        if (p.is_free) free_mem += p.size;
    }
    return free_mem;
}

// Run simulation for a specific allocation strategy
void runSimulation(int total_memory, const vector<Request>& all_requests, int strategy, const string& strategy_name) {
    cout << "\n========================================================\n";
    cout << " Strategy: " << strategy_name << "\n";
    cout << " Total Memory: " << total_memory << " KB\n";
    cout << "========================================================\n";

    vector<Partition> memory;
    memory.push_back({0, total_memory, true, -1, 0});

    vector<Request> waiting_queue;
    int req_index = 0;
    int current_time = 0;

    int total_considered = 0;
    int successfully_allocated = 0;

    while (req_index < (int)all_requests.size() || !waiting_queue.empty()) {
        // Advance time to next arrival if waiting queue is empty
        if (waiting_queue.empty() && req_index < (int)all_requests.size() && current_time < all_requests[req_index].arrival_time) {
            current_time = all_requests[req_index].arrival_time;
        }

        // Release finished processes at current time
        freeCompleted(memory, current_time);

        // Add arriving requests to waiting queue
        while (req_index < (int)all_requests.size() && all_requests[req_index].arrival_time <= current_time) {
            waiting_queue.push_back(all_requests[req_index]);
            total_considered++;
            req_index++;

            // Report statistics after every 10 requests considered
            if (total_considered % 10 == 0) {
                double success_pct = (double)successfully_allocated / total_considered * 100.0;
                double ext_frag_pct = (double)getFreeMemory(memory) / total_memory * 100.0;
                double int_frag_pct = 0.0; // Variable partitioning has 0 internal fragmentation

                cout << "\n--- Statistics after " << total_considered << " requests considered ---\n";
                cout << fixed << setprecision(2);
                cout << "  - Requests Considered     : " << total_considered << "\n";
                cout << "  - Successfully Allocated  : " << successfully_allocated << "\n";
                cout << "  - Success Percentage      : " << success_pct << " %\n";
                cout << "  - External Fragmentation  : " << ext_frag_pct << " % (" << getFreeMemory(memory) << " KB free)\n";
                cout << "  - Internal Fragmentation  : " << int_frag_pct << " %\n";
            }
        }

        // Try allocating from waiting queue in order
        bool allocated_any = false;
        for (auto it = waiting_queue.begin(); it != waiting_queue.end(); ) {
            int part_idx = findPartition(memory, it->block_size, strategy);
            if (part_idx != -1) {
                // Allocate partition
                Partition& p = memory[part_idx];
                int rem_size = p.size - it->block_size;

                p.size = it->block_size;
                p.is_free = false;
                p.pid = it->pid;
                p.end_time = current_time + it->duration;

                // Insert leftover space as new free partition
                if (rem_size > 0) {
                    Partition leftover = {p.start + it->block_size, rem_size, true, -1, 0};
                    memory.insert(memory.begin() + part_idx + 1, leftover);
                }

                successfully_allocated++;
                it = waiting_queue.erase(it);
                allocated_any = true;
            } else {
                ++it;
            }
        }

        // Advance time
        if (!allocated_any) {
            // Find earliest finishing process to advance time directly
            int next_finish = -1;
            for (const auto& p : memory) {
                if (!p.is_free) {
                    if (next_finish == -1 || p.end_time < next_finish) {
                        next_finish = p.end_time;
                    }
                }
            }

            if (req_index < (int)all_requests.size()) {
                if (next_finish != -1 && next_finish <= all_requests[req_index].arrival_time) {
                    current_time = next_finish;
                } else {
                    current_time = all_requests[req_index].arrival_time;
                }
            } else if (next_finish != -1) {
                current_time = next_finish;
            } else {
                break;
            }
        }
    }

    // Final summary
    double final_success_pct = (double)successfully_allocated / total_considered * 100.0;
    cout << "\n>>> Final Result for " << strategy_name << ":\n";
    cout << "  - Total Requests: " << total_considered << " | Allocated: " << successfully_allocated 
         << " | Success Rate: " << fixed << setprecision(2) << final_success_pct << " %\n";
}

int main(int argc, char* argv[]) {
    string filename = (argc > 1) ? argv[1] : "alloc.dat";

    ifstream infile(filename);
    if (!infile.is_open()) {
        cerr << "Error: Could not open input file " << filename << endl;
        return 1;
    }

    int total_memory;
    if (!(infile >> total_memory)) {
        cerr << "Error reading memory size from file.\n";
        return 1;
    }

    vector<Request> requests;
    int pid, arr_time, b_size, dur;
    while (infile >> pid) {
        if (pid == -1) break;
        if (infile >> arr_time >> b_size >> dur) {
            requests.push_back({pid, arr_time, b_size, dur});
        }
    }
    infile.close();

    cout << "--- OS Lab 8: Variable Partition Memory Allocation Simulator ---\n";
    cout << "Loaded " << requests.size() << " requests from " << filename << "\n";

    // Simulate all three strategies independently
    runSimulation(total_memory, requests, 0, "First Fit");
    runSimulation(total_memory, requests, 1, "Best Fit");
    runSimulation(total_memory, requests, 2, "Worst Fit");

    return 0;
}
