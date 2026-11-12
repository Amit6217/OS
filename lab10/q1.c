#include <stdio.h>
#include <stdlib.h>

typedef enum {
    ALGO_FIFO = 0,
    ALGO_LRU,
    ALGO_OPTIMAL,
    ALGO_COUNT
} AlgorithmType;

typedef struct {
    const char *name;
    int faults;
    int hits;
    int *replaced_pages;
    int replaced_count;
} SimulationResult;

static const char *algo_name(AlgorithmType algo) {
    switch (algo) {
        case ALGO_FIFO:    return "FIFO";
        case ALGO_LRU:     return "LRU";
        case ALGO_OPTIMAL: return "Optimal";
        default:           return "Unknown";
    }
}

static int find_page_in_frames(const int *frames, int frame_count, int page) {
    for (int i = 0; i < frame_count; i++) {
        if (frames[i] == page) {
            return i;
        }
    }
    return -1;
}

static int find_empty_frame(const int *frames, int frame_count) {
    return find_page_in_frames(frames, frame_count, -1);
}

static int select_lru_victim(const int *last_used, int frame_count) {
    int victim = 0;
    for (int i = 1; i < frame_count; i++) {
        if (last_used[i] < last_used[victim]) {
            victim = i;
        }
    }
    return victim;
}

static int select_optimal_victim(const int *frames, int frame_count,
                                 const int *refs, int ref_count, int current_step) {
    int victim = 0;
    int farthest_use = -1;

    for (int i = 0; i < frame_count; i++) {
        int next_use = ref_count; /* Sentinel: page is never referenced again */
        for (int j = current_step + 1; j < ref_count; j++) {
            if (frames[i] == refs[j]) {
                next_use = j;
                break;
            }
        }
        if (next_use > farthest_use) {
            farthest_use = next_use;
            victim = i;
        }
    }
    return victim;
}

static void print_frames(const int *frames, int frame_count) {
    printf("[");
    for (int i = 0; i < frame_count; i++) {
        if (frames[i] == -1) {
            printf("  -");
        } else {
            printf(" %2d", frames[i]);
        }
        if (i < frame_count - 1) {
            printf(" ");
        }
    }
    printf(" ]");
}

static SimulationResult simulate_algorithm(AlgorithmType algo,
                                           const int *refs, int ref_count,
                                           int frame_count) {
    SimulationResult res;
    res.name = algo_name(algo);
    res.faults = 0;
    res.hits = 0;
    res.replaced_count = 0;
    res.replaced_pages = (int *)malloc((size_t)ref_count * sizeof(int));

    int *frames = (int *)malloc((size_t)frame_count * sizeof(int));
    int *last_used = (int *)malloc((size_t)frame_count * sizeof(int));
    if (!res.replaced_pages || !frames || !last_used) {
        fprintf(stderr, "Memory allocation failed.\n");
        free(res.replaced_pages);
        free(frames);
        free(last_used);
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < frame_count; i++) {
        frames[i] = -1;
        last_used[i] = -1;
    }

    int fifo_ptr = 0;

    printf("\n============================================================\n");
    printf("Algorithm: %s\n", res.name);
    printf("============================================================\n");
    printf("%-6s %-6s %-*s %-8s %-10s\n",
           "Step", "Page", frame_count * 4 + 3, "Frames", "Status", "Replaced");
    printf("------------------------------------------------------------\n");

    for (int step = 0; step < ref_count; step++) {
        int page = refs[step];
        int hit_idx = find_page_in_frames(frames, frame_count, page);
        int replaced_page = -1;
        const char *status;

        if (hit_idx != -1) {
            res.hits++;
            last_used[hit_idx] = step;
            status = "Hit";
        } else {
            res.faults++;
            status = "Fault";

            int target_idx = find_empty_frame(frames, frame_count);
            if (target_idx == -1) {
                if (algo == ALGO_FIFO) {
                    target_idx = fifo_ptr;
                    fifo_ptr = (fifo_ptr + 1) % frame_count;
                } else if (algo == ALGO_LRU) {
                    target_idx = select_lru_victim(last_used, frame_count);
                } else {
                    target_idx = select_optimal_victim(frames, frame_count,
                                                       refs, ref_count, step);
                }
                replaced_page = frames[target_idx];
                res.replaced_pages[res.replaced_count++] = replaced_page;
            }

            frames[target_idx] = page;
            last_used[target_idx] = step;
        }

        printf("%-6d %-6d ", step + 1, page);
        print_frames(frames, frame_count);
        printf("  %-8s ", status);
        if (replaced_page != -1) {
            printf("%d\n", replaced_page);
        } else {
            printf("-\n");
        }
    }

    double fault_rate = (double)res.faults / ref_count * 100.0;
    double hit_rate = (double)res.hits / ref_count * 100.0;

    printf("------------------------------------------------------------\n");
    printf("Total Page Faults : %d (%.2f%%)\n", res.faults, fault_rate);
    printf("Total Page Hits   : %d (%.2f%%)\n", res.hits, hit_rate);
    printf("Pages Replaced (%d): ", res.replaced_count);
    if (res.replaced_count == 0) {
        printf("None");
    } else {
        for (int i = 0; i < res.replaced_count; i++) {
            printf("%d%s", res.replaced_pages[i],
                   (i < res.replaced_count - 1) ? ", " : "");
        }
    }
    printf("\n");

    free(frames);
    free(last_used);
    return res;
}

static void print_comparison(const SimulationResult results[ALGO_COUNT], int ref_count) {
    printf("\n====================================================================\n");
    printf("PERFORMANCE COMPARISON (References: %d)\n", ref_count);
    printf("====================================================================\n");
    printf("%-10s %-12s %-10s %-12s %-12s %-10s\n",
           "Algorithm", "Page Faults", "Page Hits", "Fault Rate", "Hit Rate", "Replaced");
    printf("--------------------------------------------------------------------\n");

    int min_faults = results[0].faults;
    for (int i = 0; i < ALGO_COUNT; i++) {
        double fault_rate = (double)results[i].faults / ref_count * 100.0;
        double hit_rate = (double)results[i].hits / ref_count * 100.0;
        printf("%-10s %-12d %-10d %9.2f%%   %9.2f%%   %-10d\n",
               results[i].name, results[i].faults, results[i].hits,
               fault_rate, hit_rate, results[i].replaced_count);
        if (results[i].faults < min_faults) {
            min_faults = results[i].faults;
        }
    }

    printf("--------------------------------------------------------------------\n");
    printf("Best Performing Algorithm(s) (Minimum %d Page Faults): ", min_faults);
    int first = 1;
    for (int i = 0; i < ALGO_COUNT; i++) {
        if (results[i].faults == min_faults) {
            if (!first) {
                printf(", ");
            }
            printf("%s", results[i].name);
            first = 0;
        }
    }
    printf("\n");
}

int main(void) {
    int frame_count;
    int ref_count;

    printf("Enter number of available page frames: ");
    if (scanf("%d", &frame_count) != 1 || frame_count <= 0) {
        fprintf(stderr, "Invalid number of frames. Must be a positive integer.\n");
        return EXIT_FAILURE;
    }

    printf("Enter length of page reference sequence: ");
    if (scanf("%d", &ref_count) != 1 || ref_count <= 0) {
        fprintf(stderr, "Invalid sequence length. Must be a positive integer.\n");
        return EXIT_FAILURE;
    }

    int *refs = (int *)malloc((size_t)ref_count * sizeof(int));
    if (!refs) {
        fprintf(stderr, "Memory allocation failed.\n");
        return EXIT_FAILURE;
    }

    printf("Enter the page reference sequence (%d non-negative integers):\n", ref_count);
    for (int i = 0; i < ref_count; i++) {
        if (scanf("%d", &refs[i]) != 1 || refs[i] < 0) {
            fprintf(stderr, "Invalid page number at position %d. Must be >= 0.\n", i + 1);
            free(refs);
            return EXIT_FAILURE;
        }
    }

    SimulationResult results[ALGO_COUNT];
    for (int i = 0; i < ALGO_COUNT; i++) {
        results[i] = simulate_algorithm((AlgorithmType)i, refs, ref_count, frame_count);
    }

    print_comparison(results, ref_count);

    for (int i = 0; i < ALGO_COUNT; i++) {
        free(results[i].replaced_pages);
    }
    free(refs);

    return EXIT_SUCCESS;
}
