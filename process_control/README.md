# Process Control in C — `fork()`, `exec()`, `wait()`, and `exit()`

This folder contains 13 small C programs designed to understand **process control in Linux/Unix** by running them and observing their output.

Compile and run each program from the terminal:

```bash
gcc 1.c -o 1
./1
```

> Output order involving multiple processes is often **not deterministic**. Run programs multiple times.

---

## 1. `fork()` — Create a New Process

### Core idea

```c
fork();
```

creates a **new child process**.

After `fork()`, both parent and child continue execution from the **next statement**.

```text
Before fork:
        Parent

After fork:
        Parent
          |
        Child
```

### Return value of `fork()`

```text
Parent → child's PID (> 0)
Child  → 0
Failure → -1
```

Therefore:

```c
int pid = fork();

if (pid == 0)
    // child
else if (pid > 0)
    // parent
else
    // fork failed
```

---

## 2. Parent and Child Have Separate Memory

After `fork()`, the child initially gets a copy of the parent's address space.

```text
Parent memory          Child memory
-------------          -------------
x = 10                 x = 10
   ↓                       ↓
x = 11                 x = 11
```

Changing a normal variable in one process does **not** change it in the other.

Modern systems implement this efficiently using **copy-on-write (COW)**.

---

## 3. Multiple `fork()` Calls

For independent `fork()` calls:

```c
fork();
fork();
```

the number of processes becomes:

```text
Initial       1
1st fork      2
2nd fork      4
```

For `n` independent forks:

```text
Number of processes = 2^n
```

But be careful: this rule applies only when **every existing process executes every fork**. Conditions such as `if`, early `exit()`, or `exec()` can change the count.

---

## 4. Process Execution Order Is NOT Guaranteed

After `fork()`, parent and child are independently scheduled by the OS.

For example:

```text
Run 1:
Parent
Child

Run 2:
Child
Parent
```

Both are correct.

### Important

`fork()` guarantees that a child is created.

It does **not** guarantee which process executes first.

The **CPU scheduler** decides when each runnable process gets CPU time.

---

## 5. `getpid()` and `getppid()`

Useful functions for observing processes:

```c
getpid()
```

→ returns the PID of the current process.

```c
getppid()
```

→ returns the PID of the current process's parent.

Example:

```text
PID = 1542, PPID = 1541
```

means:

```text
1541 → parent
1542 → child
```

---

## 6. Orphan Process

An **orphan process** occurs when its parent terminates before the child.

Example:

```text
Parent (1541)
     |
     └── Child (1542)

Parent terminates
     ↓

Child (1542)
     |
     └── gets adopted/reparented by a system process
```

If the child later calls:

```c
getppid()
```

it may see a different PPID because its original parent has already terminated.

This is why output such as:

```text
PID = 1542, PPID = 814
```

can occur even though the child was originally created by PID 1541.

### Key distinction

```text
Parent terminates first → ORPHAN
Child terminates first   → may become ZOMBIE
```

---

## 7. `wait()` — Parent Waits for a Child

Basic form:

```c
wait(NULL);
```

means:

> Wait for a child process to terminate, but ignore its termination status.

Without `wait()`:

```text
Parent ─────────→ may finish first
Child  ─────────────────→ finishes later
```

With `wait()`:

```text
Parent ── wait ─────────────→ continues
             ↑
Child ───────┘
       terminates
```

`wait()` is mainly used for **synchronization** and for collecting a child's termination information.

---

## 8. What Does `NULL` Mean in `wait(NULL)`?

The declaration is essentially:

```c
pid_t wait(int *status);
```

The argument is a pointer where the OS can store information about the child's termination.

### `wait(NULL)`

```c
wait(NULL);
```

means:

> Wait for a child, but I don't need its termination status.

### `wait(&status)`

```c
int status;
wait(&status);
```

means:

> Wait for a child and store its termination information in `status`.

Here:

```c
&status
```

means **the memory address of `status`**.

---

## 9. Child Exit Status

A child can terminate with:

```c
exit(5);
```

The parent can collect this information:

```c
int status;

wait(&status);

printf("%d\n", WEXITSTATUS(status));
```

This prints:

```text
5
```

Do NOT assume the raw value returned in `status` itself is the exit code. The status contains **encoded termination information**.

Useful macros:

```c
WIFEXITED(status)
```

→ Was the child normally terminated?

```c
WEXITSTATUS(status)
```

→ What exit code did the child return?

Other useful macros include:

```c
WIFSIGNALED(status)
WTERMSIG(status)
```

for termination by signal.

---

## 10. `exit()` — Terminate the Current Process

```c
exit(0);
```

terminates the current process immediately.

Code after it is not executed:

```c
printf("Before");
exit(0);
printf("After");    // never executes
```

### Common convention

```text
exit(0)     → successful termination
exit(nonzero) → error/failure or other status
```

The exit status can be collected by the parent using `wait()` / `waitpid()`.

---

## 11. `exec()` — Replace the Current Program

`exec()` is fundamentally different from `fork()`.

### `fork()`

Creates another process:

```text
Before:
     P

After:
     P
     C
```

### `exec()`

Does **not** create a new process.

It replaces the current process's program:

```text
Before:
Process
└── your_program

exec("ls")

After:
Process
└── ls
```

The PID normally remains the same.

If `exec()` succeeds, the old program's code is replaced, so statements after `exec()` are **not executed**.

Example:

```c
printf("Before exec\n");

execlp("ls", "ls", "-l", NULL);

printf("After exec\n");
```

Normally:

```text
Before exec
<ls output>
```

No:

```text
After exec
```

If the `printf` after `exec()` executes, `exec()` failed.

---

## 12. `fork() + exec()` — Very Important Pattern

A very common Unix process-control pattern is:

```text
              fork()
             /      \
            /        \
       Parent        Child
          |            |
        wait()       exec()
                       |
                       ↓
                  New program
                       |
                       ↓
                    exits
                       |
                       ↓
                    Parent
                    continues
```

This is conceptually similar to how a shell runs another program.

For example, when you type:

```bash
ls
```

a shell can conceptually:

```text
Shell
  |
 fork()
  |
  +----------------+
  |                |
Parent           Child
  |                |
wait()           exec("ls")
                   |
                   ↓
                  ls
```

---




## 13. Orphan Process Demo

Program `12.c` demonstrates an **orphan process**.

The parent exits immediately after `fork()`. The child sleeps for 3 seconds and then prints its PPID again.

```text
Run 1:
Parent (PID = 1541): exiting immediately
Child: PID = 1542, PPID = 1541
Child after sleep: PID = 1542, PPID = 814   ← reparented!
```

The second PPID is different because:

1. The parent terminated during the `sleep(3)`.
2. The child became an orphan.
3. A system process adopted the child.

### How to observe

Run the program and compare the two PPID values printed by the child. If they differ, you have witnessed reparenting.

---

## 14. Zombie Process Demo

Program `13.c` demonstrates **zombie processes** using a `for` loop.

The parent creates **5 children** in a loop. Each child exits immediately after printing its PID. The parent then sleeps for 30 seconds **without calling `wait()`**.

During that 30-second window, all 5 children exist as zombies — they have terminated, but their process-table entries remain because the parent hasn't collected their termination status.

```text
Child 1 (PID = 2001): exiting now
Child 2 (PID = 2002): exiting now
Child 3 (PID = 2003): exiting now
Child 4 (PID = 2004): exiting now
Child 5 (PID = 2005): exiting now

Parent (PID = 2000): created 5 children
Parent: sleeping for 30 seconds WITHOUT calling wait()...
```

### How to observe

1. Run `13.c` in one terminal.
2. In another terminal, run:

```bash
ps -e -o pid,ppid,stat,comm | grep Z
```

You should see **5 processes** listed with state `Z+` (zombie). This makes it very clear compared to a single zombie.

After the parent exits (or after 30 seconds), all zombies are cleaned up.

---

# The Four System Calls — Memorize This

| System call | Main purpose |
|---|---|
| `fork()` | **Create a new process** |
| `exec()` | **Replace current process's program** |
| `wait()` | **Wait for child termination / collect status** |
| `exit()` | **Terminate current process** |

The most important conceptual distinction:

```text
fork()  → creates a process
exec()  → changes what a process is running
wait()  → synchronizes parent with child
exit()  → terminates a process
```

---

# Process Lifecycle

A simplified process lifecycle:

```text
             fork()
               |
               ↓
        ┌─────────────┐
        │   CREATED   │
        └──────┬──────┘
               ↓
             RUNNING
               |
          exec() optional
               |
               ↓
          New program
               |
             exit()
               ↓
          TERMINATED
               |
          parent wait()
               ↓
          termination
          information
          collected
```

---

# Orphan vs Zombie

This is an important exam topic.

## Orphan

Parent terminates first:

```text
Parent
   X
   |
 Child
   ↓
Orphan
```

The child is reparented to a system process.

## Zombie

Child terminates first, but parent has not collected its termination status:

```text
Parent
   |
 Child
   X
   ↓
Zombie
```

The child is no longer running, but a small process-table entry remains until the parent calls `wait()`/`waitpid()` or otherwise handles the child.

### Easy memory trick

```text
ORPHAN  → Parent is gone
ZOMBIE  → Child is gone, but not collected
```

---

# `wait()` vs `waitpid()`

### `wait()`

```c
wait(&status);
```

Waits for a child process to terminate.

### `waitpid()`

```c
waitpid(pid, &status, 0);
```

Can wait for a **specific child PID**.

Conceptually:

```text
wait()             → wait for a child
waitpid(childPID)  → wait for this specific child
```

The third argument of `waitpid()` controls its behavior/options.

---

# Important Observations While Running the Programs

When testing these programs, deliberately run them multiple times.

Look for:

### 1. Different output order

```text
Parent
Child
```

vs.

```text
Child
Parent
```

This demonstrates **nondeterministic scheduling**.

### 2. Different PIDs

Every execution creates processes with different PIDs.

### 3. Different PPIDs

If a parent terminates before a child calls `getppid()`, you may observe **orphan/reparenting behavior**.

### 4. `exec()` stops the old program

If `exec()` succeeds, code after it doesn't execute.

### 5. `wait()` changes ordering

The parent can be forced to wait for a child before continuing.

### 6. `exit()` stops the current process

Statements after `exit()` are not executed.

---

# Important Header Files

For these experiments:

```c
#include <stdio.h>      // printf()
#include <unistd.h>     // fork(), exec(), getpid(), getppid(), sleep()
#include <sys/wait.h>   // wait(), waitpid(), WIFEXITED(), WEXITSTATUS()
#include <stdlib.h>     // exit()
```

---

# Common Mistakes

### Mistake 1: Thinking `exec()` creates a process

Wrong:

```text
exec() → new process
```

Correct:

```text
fork() → new process
exec() → replace program in existing process
```

### Mistake 2: Assuming output order after `fork()`

Wrong:

> Parent always runs first.

Correct:

> The scheduler decides the execution order.

### Mistake 3: Thinking `wait(NULL)` means wait for PID NULL

Wrong.

`NULL` means:

> Don't provide a location for receiving the child's termination status.

### Mistake 4: Thinking `exit()` only exits a function

`exit()` terminates the **entire current process**, not merely `main()` or a function.

---

# One Final Mental Model

If you remember only this, remember:

```text
              fork()
                |
        ┌───────┴────────┐
        ↓                ↓
     Parent             Child
        |                 |
      wait()             exec()
        |                 |
        |                 ↓
        |            New program
        |                 |
        |               exit()
        |                 |
        └───────←─────────┘
          child terminates
```

### Four words to remember

```text
fork  = CREATE
exec  = REPLACE
wait  = WAIT/COLLECT
exit  = TERMINATE
```

---

## Recommended order for studying

Run the files in this order:

```text
1 → fork basics
2 → fork return value
3 → separate memory
4 → multiple fork
5 → wait
6 → exit
7 → exit status
8 → exec
9 → fork + exec
10 → scheduling
11 → wait + scheduling
12 → orphan process
13 → zombie process
```

After finishing these, the next useful step is to practice **`fork()` questions involving `if`, `&&`, `||`, loops, `wait()`, and `exit()`**, because those are the patterns commonly used in OS/placement/exam questions.
