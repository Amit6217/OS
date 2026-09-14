# Lab 6 — Process Synchronization & Concurrency

## 📚 Theory & Core Concepts

### 1. The Critical Section Problem

When multiple processes or threads access **shared data** concurrently, we get a **race condition** —
the outcome depends on the unpredictable order of execution. The region of code that accesses
shared data is called the **critical section**.

```
Process A                    Process B
─────────                    ─────────
read  count  → 5             read  count  → 5
count = 5 + 1                count = 5 + 1
write count  → 6             write count  → 6    ← LOST UPDATE! Should be 7
```

The **Critical Section Problem** requires a solution that satisfies three conditions:

| Property | Meaning |
|----------|---------|
| **Mutual Exclusion** | At most one process is in the critical section at any time |
| **Progress** | If no process is in the critical section and some want to enter, one must be allowed in — no deadlock |
| **Bounded Waiting** | No process waits forever to enter — no starvation |

---

### 2. Busy-Waiting (Spinlock) Solutions

These are **software-only** solutions that use shared variables and loops (`while(condition);`)
to achieve mutual exclusion without hardware or OS support.

---

#### 2a. Peterson's Algorithm (2 Processes)

The simplest correct solution for 2 processes. Uses two shared variables:

```c
volatile int flag[2] = {false, false};  // "I want to enter"
volatile int turn;                       // "It's your turn" (courtesy)
```

**Entry protocol** for process `i` (where `j = 1 - i`):
```c
flag[i] = true;      // I want to enter
turn = j;            // But I'll let you go first
while (flag[j] && turn == j);  // Wait if you also want in AND it's your turn
// → ENTER critical section
```

**Exit protocol**:
```c
flag[i] = false;     // I'm done, I no longer want in
```

**Why it works**:
- **Mutual exclusion**: If both want in, `turn` can only be 0 OR 1, so one must proceed.
- **Progress**: A process that doesn't want in (`flag[j] = false`) never blocks the other.
- **Bounded waiting**: After exiting, process `i` sets `turn = j`, giving the other priority.

**Limitation**: Only works for exactly **2** processes.

---

#### 2b. Dekker's Algorithm (2 Processes)

The **first known correct solution** (1965) to the mutual exclusion problem for 2 processes.
Uses the same variables as Peterson's but with a different (more complex) protocol:

```c
volatile bool flag[2] = {false, false};
volatile int turn = 0;
```

**Entry protocol** for process `i`:
```c
flag[i] = true;
while (flag[j]) {           // If the other also wants in...
    if (turn == j) {         // ...and it's their turn...
        flag[i] = false;     // ...back off (retract my intention)
        while (turn == j);   // ...wait until it's my turn
        flag[i] = true;      // ...then re-declare my intention
    }
}
```

**Exit protocol**:
```c
turn = j;           // Give the other process priority next time
flag[i] = false;     // I'm done
```

**Key difference from Peterson's**: Dekker's uses a **back-off** mechanism — if there's contention,
the lower-priority process retracts its flag and waits. Peterson's is simpler because the
`turn` variable alone resolves contention.

---

#### 2c. Lamport's Bakery Algorithm (N Processes)

Generalizes mutual exclusion to **any number of processes** (1974, by Leslie Lamport).
Named after a bakery where customers take numbered tickets:

```c
volatile bool choosing[N] = {false};  // "I'm picking a number"
volatile int  number[N]   = {0};      // My ticket number (0 = not waiting)
```

**Entry protocol** for process `i`:
```c
choosing[i] = true;
number[i] = 1 + max(number[0], number[1], ..., number[N-1]);
choosing[i] = false;

for (j = 0; j < N; j++) {
    while (choosing[j]);   // Wait while j is picking a number
    while (number[j] != 0 && (number[j], j) < (number[i], i));
    // Wait if j has a lower ticket, or same ticket but lower ID
}
```

**Exit protocol**:
```c
number[i] = 0;   // I'm done
```

**Why `choosing` is needed**: Without it, process A might read process B's half-written ticket
number (B is in the middle of calculating it), see a smaller value, and both enter.

**The `(number[j], j) < (number[i], i)` comparison**: This is a **lexicographic comparison** —
first compare ticket numbers, and if equal, compare process IDs. This guarantees a unique
ordering even when two processes pick the same number.

---

### 3. The Producer-Consumer Problem

A classic synchronization problem:

```
Producer → [ Bounded Buffer (size N) ] → Consumer
```

- **Producer** generates items and places them in the buffer.
- **Consumer** removes items from the buffer.
- Buffer is **bounded** — producer must wait if full, consumer must wait if empty.
- **Mutual exclusion** is needed so both don't modify the buffer simultaneously.

```c
// Shared variables
int buffer[N];
int in = 0;    // Next position to write (producer)
int out = 0;   // Next position to read (consumer)
int count = 0; // Current number of items

// Producer
while (count == N);     // Wait if buffer full
buffer[in] = item;
in = (in + 1) % N;
count++;

// Consumer
while (count == 0);     // Wait if buffer empty
item = buffer[out];
out = (out + 1) % N;
count--;
```

Without synchronization, `count++` and `count--` are **not atomic** — they involve read-modify-write,
leading to race conditions.

---

### 4. Shared Memory Between Processes

Unlike threads (which share the same address space), **processes** have isolated memory.
To share data between `fork()`ed processes, we need explicit shared memory:

#### Using `mmap` (Modern approach):
```c
#include <sys/mman.h>

shared_data_t *shared = mmap(
    NULL,                           // Let kernel choose address
    sizeof(shared_data_t),          // Size
    PROT_READ | PROT_WRITE,         // Read and write access
    MAP_SHARED | MAP_ANONYMOUS,     // Shared between processes, no file backing
    -1, 0                           // No file descriptor
);
```

**`MAP_SHARED`** is the critical flag — without it, each process gets a private copy (COW),
and changes are invisible to the other.

#### Using `shmget`/`shmat` (System V approach):
```c
int shmid = shmget(IPC_PRIVATE, size, IPC_CREAT | 0666);
void *ptr = shmat(shmid, NULL, 0);
```

---

### 5. POSIX Threads (pthreads)

Threads are **lightweight processes** that share the same address space. Unlike `fork()`,
threads share:
- Global variables, heap, and file descriptors
- Code segment

Each thread has its own:
- Stack (local variables)
- Program counter
- Register set

```c
#include <pthread.h>

void* thread_function(void* arg) {
    // Thread body
    return NULL;
}

int main() {
    pthread_t thread;
    pthread_create(&thread, NULL, thread_function, arg);  // Start thread
    pthread_join(thread, NULL);                           // Wait for thread
}
```

**Compile with**: `gcc -o program program.c -lpthread`

---

### 6. Mutex Locks (Mutual Exclusion)

A **mutex** is an OS/library-provided lock for protecting critical sections in threaded programs:

```c
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Or dynamically:
pthread_mutex_t lock;
pthread_mutex_init(&lock, NULL);

// Usage:
pthread_mutex_lock(&lock);      // Acquire (blocks if already held)
// ... critical section ...
pthread_mutex_unlock(&lock);    // Release

pthread_mutex_destroy(&lock);   // Clean up
```

**Key properties**:
- **Only the owner can unlock** — the thread that locked must be the one to unlock.
- **Blocking** — if the lock is held, `pthread_mutex_lock()` puts the calling thread to sleep
  (unlike busy-waiting, this doesn't waste CPU cycles).
- **Per-resource locking** — in Q3 (bank), each account has its own mutex, so transactions
  on different accounts can proceed in parallel.

**Mutex vs Busy-Waiting**:
| Feature | Mutex | Busy-Wait (Peterson/Bakery) |
|---------|-------|---------------------------|
| CPU usage while waiting | Sleeps (efficient) | Spins in a loop (wasteful) |
| Implementation | OS/library provided | Software-only |
| Suitable for | Real applications | Academic study |
| Overhead | Context switch cost | None (but wastes CPU) |

---

### 7. Semaphores

A **semaphore** is a synchronization primitive with an integer value and two atomic operations:

```
sem_wait(S):  while (S <= 0) sleep;  S--;    // Also called P() or down()
sem_post(S):  S++;  wake_one_sleeper();       // Also called V() or up()
```

**Types**:
- **Binary semaphore** (value 0 or 1) — behaves like a mutex.
- **Counting semaphore** (value 0 to N) — controls access to N identical resources.

**POSIX Named Semaphores** (work across processes via `fork()`):
```c
#include <semaphore.h>
#include <fcntl.h>

sem_t *sem = sem_open("/my_sem", O_CREAT, 0666, initial_value);
sem_wait(sem);     // Decrement (block if 0)
sem_post(sem);     // Increment (wake a waiter)
sem_close(sem);    // Close handle
sem_unlink("/my_sem");  // Remove from system
```

**In Q4**: The semaphore starts at 0, so `sem_wait()` in the parent **blocks** until the child
calls `sem_post()`. This guarantees child-before-parent execution order.

---

### 8. The `volatile` Keyword

```c
volatile int flag = 0;
```

`volatile` tells the compiler: **"Don't optimize accesses to this variable — always read from
memory, never cache it in a register."**

Without `volatile`, the compiler might optimize:
```c
while (flag == 0);  →  Compiler: "flag never changes in this loop, so this is infinite. Optimize away!"
```

Essential for **all shared variables** used in busy-waiting synchronization algorithms.

---

## 🔬 Assignment Design Breakdown

### Q1: Classical Algorithms (3 Programs)

All three implement the **same problem** (producer-consumer with bounded buffer between
`fork()`ed processes) using different mutual exclusion algorithms:

| Program | Algorithm | Complexity | Historical Significance |
|---------|-----------|------------|------------------------|
| `q1_peterson.c` | Peterson's | Simple | Simplified Dekker's (1981) |
| `q1_dekker.c` | Dekker's | Medium | First correct 2-process solution (1965) |
| `q1_lamport.c` | Lamport's Bakery | Complex | First correct N-process solution (1974) |

All use `mmap(MAP_SHARED|MAP_ANONYMOUS)` for shared memory and `volatile` for shared flags.

---

### Q2: Producer-Consumer Variants (1 Program, 4 Modes)

Demonstrates how synchronization scales from 2 to N threads:

| Variant | Threads | Algorithm | Why |
|---------|---------|-----------|-----|
| 1P-1C | 2 | Peterson's | Only 2 threads → Peterson's works directly |
| 1P-3C | 4 | Lamport's Bakery | >2 threads → need N-process algorithm |
| 3P-1C | 4 | Lamport's Bakery | Same reason |
| 3P-3C | 6 | Lamport's Bakery | Same reason |

**Key insight**: Peterson's is limited to 2 processes. Lamport's Bakery is the natural
generalization for N processes, maintaining all three correctness properties.

---

### Q3: Bank Simulation (Mutex)

Demonstrates **real-world use** of mutex locks:

```
Account 0 [mutex_0]  ←── Client threads randomly pick accounts
Account 1 [mutex_1]  ←── and do deposits/withdrawals
Account 2 [mutex_2]
```

**Per-account locking**: Each account has its own mutex. This is better than a single global lock
because transactions on **different** accounts can happen in parallel (fine-grained locking).

**Invariant**: The total money in the system should equal the sum of initial balances + all
successful deposits - all successful withdrawals. Mutex ensures this invariant is maintained.

---

### Q4: Semaphore Ordering (fork + semaphore)

The simplest demonstration of **process ordering with semaphores**:

```
Semaphore value: 0  (initialized)

Child:                          Parent:
  VAR -= 2                       sem_wait(sem)  ← BLOCKS (sem is 0)
  print VAR                      ... waiting ...
  sem_post(sem) ← sem becomes 1  ... wakes up ...
  exit                            VAR += 2
                                  print VAR
```

This is a fundamental pattern: **sem initialized to 0** creates a "happens-before" relationship.

---

### Q5: Multithreading Basics

The simplest multithreading program — three threads computing independently on shared data:

```
Main Thread
├── Thread 1: sum(data[0..9])   → sum_result
├── Thread 2: max(data[0..9])   → max_result
├── Thread 3: min(data[0..9])   → min_result
└── join all → print results
```

**No mutex needed** because each thread writes to a **different** global variable.
There's no shared mutable state between threads — this is the ideal case.

---

## 🧠 What You Should Understand After This Lab

### Fundamental Concepts
1. **Race conditions** — why concurrent access to shared data without synchronization produces incorrect results.
2. **The three requirements** of a correct critical section solution: mutual exclusion, progress, bounded waiting.
3. **Busy-waiting vs blocking** — spinlocks waste CPU but have no context-switch overhead; mutexes/semaphores sleep efficiently.

### Algorithms
4. **Peterson's Algorithm** — `flag[]` declares intent, `turn` provides courtesy. Only works for 2 processes.
5. **Dekker's Algorithm** — the first correct solution; uses back-off when both want in.
6. **Lamport's Bakery** — ticket-based, works for N processes. The `choosing` flag prevents reading half-written tickets.

### Synchronization Primitives
7. **Mutex** — a lock for threads. Acquire before critical section, release after. Per-resource locking allows parallelism.
8. **Semaphore** — a generalized counter. Binary semaphore ≈ mutex. Initialized to 0 → enforces ordering.

### Systems Programming
9. **Shared memory** — `mmap(MAP_SHARED)` or `shmget/shmat` makes memory visible across `fork()`ed processes.
10. **`volatile`** — prevents compiler from caching shared variables in registers during busy-wait loops.
11. **pthreads** — `pthread_create`, `pthread_join`, `pthread_mutex_lock/unlock` for thread management.

### Design Patterns
12. **Producer-Consumer with bounded buffer** — the classic synchronization problem.
13. **Ordering via semaphores** — initialize to 0, post after the event that must happen first.
14. **Fine-grained locking** — one lock per resource (not one global lock) maximizes concurrency.

---

## 🔧 Compilation & Execution

```bash
# Q1: Classical algorithms (no threads — uses fork)
gcc -o q1_dekker q1_dekker.c && ./q1_dekker
gcc -o q1_lamport q1_lamport.c && ./q1_lamport
gcc -o q1_peterson q1_peterson.c && ./q1_peterson

# Q2: Producer-consumer variants (threads)
gcc -o q2_producer_consumer q2_producer_consumer.c -lpthread && ./q2_producer_consumer

# Q3: Bank simulation (threads + mutex)
gcc -o q3_bank q3_bank.c -lpthread && ./q3_bank

# Q4: Fork + semaphore
gcc -o q4_semaphore q4_semaphore.c -lpthread && ./q4_semaphore

# Q5: Multithreading
gcc -o q5_threads q5_threads.c -lpthread && ./q5_threads
```

> **Note**: All programs require Linux or WSL. The `-lpthread` flag links the POSIX threads library.

---

## 📁 Files

| File | Question | Description |
|------|----------|-------------|
| `q1_dekker.c` | Q1 | Producer-Consumer with Dekker's Algorithm |
| `q1_lamport.c` | Q1 | Producer-Consumer with Lamport's Bakery Algorithm |
| `q1_peterson.c` | Q1 | Producer-Consumer with Peterson's Algorithm |
| `q2_producer_consumer.c` | Q2 | 4 Producer-Consumer variants (menu-driven) |
| `q3_bank.c` | Q3 | Bank account simulation with mutex locks |
| `q4_semaphore.c` | Q4 | Parent-child ordering with POSIX semaphore |
| `q5_threads.c` | Q5 | Multithreaded sum, max, min computation |
| `README.md` | — | This file |
