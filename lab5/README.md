# Lab 5 — Inter-Process Communication (IPC) Using Pipes

## 📚 Theory & Core Concepts

### 1. What is Inter-Process Communication (IPC)?

In a multitasking operating system, processes often need to **exchange data** with each other.
Since each process has its own **isolated address space** (thanks to virtual memory), one process
**cannot** directly read or write another process's memory. IPC mechanisms provide controlled
channels for processes to communicate.

Common IPC mechanisms in Unix/Linux:
| Mechanism       | Direction     | Relationship     | Persistence |
|-----------------|---------------|------------------|-------------|
| **Pipe**        | Unidirectional| Parent ↔ Child   | Temporary   |
| Named Pipe (FIFO) | Unidirectional | Any processes | File-system based |
| Shared Memory   | Bidirectional | Any processes    | Kernel-managed |
| Message Queue   | Bidirectional | Any processes    | Kernel-managed |
| Socket          | Bidirectional | Any (even networked) | Flexible |
| Signal          | Notification  | Any processes    | Asynchronous |

This assignment focuses on **unnamed (anonymous) pipes** — the simplest and most fundamental IPC mechanism.

---

### 2. Pipes — How They Work

A **pipe** is a unidirectional communication channel maintained by the kernel. It works like a
**byte stream** buffer:

```
Writer Process  ──write()──►  [ Kernel Buffer (Pipe) ]  ──read()──►  Reader Process
```

Key properties:
- **Unidirectional**: Data flows in one direction only. For two-way communication, you need **two pipes**.
- **FIFO order**: Data is read in the same order it was written (First-In, First-Out).
- **Blocking**: `read()` blocks if the pipe is empty; `write()` blocks if the pipe is full.
- **Byte-oriented**: Pipes carry raw bytes — there's no concept of "messages" or boundaries.
- **Kernel-buffered**: The OS kernel manages an internal buffer (typically 64KB on Linux).

#### The `pipe()` System Call

```c
#include <unistd.h>

int fd[2];
pipe(fd);
// fd[0] → Read end  (you read FROM this)
// fd[1] → Write end (you write TO this)
```

**Critical rule**: After `fork()`, each process must **close the pipe ends it doesn't use**.
If the write end is never closed, the reader will **block forever** waiting for more data
(it will never get an EOF).

```
Before fork():
  Parent has:  fd[0] (read)  and  fd[1] (write)

After fork():
  Parent has:  fd[0] (read)  and  fd[1] (write)   ← copies of the same FDs
  Child has:   fd[0] (read)  and  fd[1] (write)

If parent is the writer and child is the reader:
  Parent closes fd[0]  (doesn't need to read)
  Child  closes fd[1]  (doesn't need to write)
```

---

### 3. Process Creation with `fork()`

`fork()` creates a **new process** (child) that is an almost-exact copy of the calling process (parent).

```c
pid_t pid = fork();

if (pid < 0)  → Fork failed (error)
if (pid == 0) → You are in the CHILD process
if (pid > 0)  → You are in the PARENT process (pid = child's PID)
```

What gets copied:
- Code, data, stack segments (via Copy-On-Write)
- Open file descriptors (including pipe FDs!)
- Environment variables

What is **not** shared:
- Address space (each has its own copy)
- PID (child gets a new PID)
- Return value of `fork()` (that's how you tell them apart)

---

### 4. Closing Unused Pipe Ends — Why It Matters

This is the **most common mistake** students make. Consider:

```
Pipe 1: Process 1 (write) → Process 2 (read)
```

After both `fork()` calls, **all three processes** hold copies of Pipe 1's file descriptors.
If Process 2 and Process 3 don't close their write end of Pipe 1, then when Process 1 finishes
writing, the reader (Process 2) **will never see EOF** because the kernel thinks there are still
writers (the unclosed write FDs in other processes).

**Rule of thumb**: In each process, immediately close every pipe end that process won't use.

---

### 5. The `read()` and `write()` System Calls

```c
ssize_t read(int fd, void *buf, size_t count);
// Returns: number of bytes read, 0 on EOF, -1 on error

ssize_t write(int fd, const void *buf, size_t count);
// Returns: number of bytes written, -1 on error
```

- `read()` returns 0 when **all write ends** of the pipe are closed → this signals EOF.
- `write()` to a pipe with no readers generates a `SIGPIPE` signal (broken pipe).

---

### 6. Waiting for Child Processes

```c
#include <sys/wait.h>

waitpid(child_pid, NULL, 0);  // Wait for a specific child
wait(NULL);                    // Wait for any child
```

The parent **must** wait for its children to:
1. Prevent **zombie processes** (terminated children whose exit status hasn't been collected).
2. Ensure the parent reads from Pipe 3 only after Process 3 has finished writing.

---

## 🔬 Assignment Design Breakdown

### Communication Architecture

```
  Process 1                 Process 2                 Process 3
  (File Reader)             (Space Formatter)         (Case Converter + Counter)
       |                         |                         |
       |----  Pipe 1 (text) ---->|                         |
       |                         |----  Pipe 2 (text) ---->|
       |                         |                         |
       |<------------- Pipe 3 (word count) ----------------|
       |
  Prints word count
```

### Three pipes, three processes:

| Pipe   | Writer     | Reader     | Data Transferred |
|--------|------------|------------|------------------|
| Pipe 1 | Process 1  | Process 2  | Raw file contents (string) |
| Pipe 2 | Process 2  | Process 3  | Formatted text (string) |
| Pipe 3 | Process 3  | Process 1  | Word count (integer) |

### What Each Process Does:

**Process 1 (Parent — File Reader)**:
- Prompts user for a filename
- Opens and reads the entire file into a buffer
- Writes the raw text into Pipe 1
- Reads an integer (word count) back from Pipe 3
- Prints the total word count

**Process 2 (Child 1 — Text Formatter)**:
- Reads raw text from Pipe 1
- Applies formatting rules:
  - Collapses multiple spaces/tabs/newlines into a single space
  - Removes spaces before punctuation (`. , ; : ! ?`)
  - Ensures exactly one space after punctuation
- Writes cleaned text into Pipe 2

**Process 3 (Child 2 — Case Converter + Word Counter)**:
- Reads formatted text from Pipe 2
- Title-cases every word (first letter uppercase, rest lowercase)
- Counts total words
- Prints the final formatted text to stdout
- Writes the word count (as a raw `int`) into Pipe 3

---

## 🧠 What You Should Understand After This Lab

1. **How `pipe()` creates a kernel-managed byte stream** between related processes.
2. **Why closing unused pipe ends is critical** — without it, `read()` never returns 0 (EOF).
3. **The `fork()` model** — child inherits copies of all open file descriptors.
4. **Unidirectional nature of pipes** — you need N pipes for N communication channels.
5. **Blocking I/O** — `read()` waits for data, `write()` waits for buffer space.
6. **Binary vs text data over pipes** — the word count is sent as raw `sizeof(int)` bytes, not as a string.
7. **Process synchronization via pipes** — the read/write blocking behavior naturally synchronizes the processes without explicit locks.
8. **Zombie prevention** — `waitpid()` is essential to clean up child processes.

---

## 🔧 Compilation & Execution

```bash
gcc -o assignment5 assignment5.c
./assignment5
# When prompted, enter: sample_input.txt
```

### Expected Output (with sample_input.txt):

```
Enter the input filename: sample_input.txt

--- Final Formatted Text ---
Hello World, This Is A Test. Operating Systems Is Fun!
How Are You Doing Today? I Hope Everything Is Going Well.
The Quick Brown Fox Jumps Over The Lazy Dog.
----------------------------
Total number of words: 27
```

---

## 📁 Files

| File | Description |
|------|-------------|
| `assignment5.c` | Main program — 3 processes, 3 pipes |
| `sample_input.txt` | Test input with bad spacing and mixed case |
| `README.md` | This file |
