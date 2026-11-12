# Lab 10 — Page Replacement Algorithms

This lab simulates and compares three virtual memory page replacement algorithms in C (`q1.c`):
1. **FIFO** (First-In, First-Out)
2. **LRU** (Least Recently Used)
3. **Optimal** (Belady's Optimal Page Replacement)

---

## Compilation and Execution

```bash
gcc -Wall -Wextra -std=c11 q1.c -o q1
./q1
```

On Windows (PowerShell with MinGW/MSYS2 GCC):

```powershell
gcc -Wall -Wextra -std=c11 q1.c -o q1.exe
.\q1.exe
```

---

## Sample Input

```text
3
20
7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1
```

- **Available page frames:** `3`
- **Reference sequence length:** `20`
- **Page reference sequence:** `7 0 1 2 0 3 0 4 2 3 0 3 2 1 2 0 1 7 0 1`

---

## Expected Summary

| Algorithm | Page Faults | Page Hits | Fault Rate | Hit Rate | Pages Replaced |
| :--- | :---: | :---: | :---: | :---: | :--- |
| **FIFO** | 15 | 5 | 75.00% | 25.00% | `7, 0, 1, 2, 3, 0, 4, 2, 3, 0, 1, 2` (12) |
| **LRU** | 12 | 8 | 60.00% | 40.00% | `7, 1, 2, 3, 0, 4, 0, 3, 2` (9) |
| **Optimal** | 9 | 11 | 45.00% | 55.00% | `7, 1, 0, 4, 3, 2` (6) |
