# Lab 9 — Paging and Address Translation

This lab covers memory management techniques:
1. **Single-Level Paging** (`q1.cpp`)
2. **Two-Level Hierarchical Paging** (`q2.cpp`)

---

## 🛠️ Compilation Commands

### Using G++ on Windows (MinGW) or Linux / WSL:
```bash
# Compile Q1
g++ q1.cpp -o q1

# Compile Q2
g++ q2.cpp -o q2
```

---

## 📘 Question 1: Single-Level Paging (`q1.cpp`)

### Overview
This program calculates:
- Number of pages in logical memory = $\text{Logical Size} / \text{Page Size}$
- Number of frames in physical memory = $\text{Physical Size} / \text{Page Size}$
- Constructs the **Page Table** (Page, Page Address, Frame Number, Presence Bit).
- Constructs the **Frame Table** (Frame, Frame Address, Page Number).
- Translates **Logical Address** to **Physical Address**:
  $$\text{Page Number} = \lfloor\text{LA} / \text{Page Size}\rfloor, \quad \text{Offset} = \text{LA} \pmod{\text{Page Size}}$$
  $$\text{Physical Address} = \text{Base} + (\text{Frame Number} \times \text{Page Size}) + \text{Offset}$$
- Detects **Page Fault** when a page is not present in physical memory.

---

### ⌨️ Inputs to Enter While Running (Q1)

When prompted, enter the following values step by step:

| Prompt | What to Enter | Reason / Explanation |
| :--- | :---: | :--- |
| `Enter logical memory size (bytes):` | `16` | Total process address space (16 bytes) |
| `Enter physical memory size (bytes):` | `32` | Total RAM size (32 bytes) |
| `Enter page size (bytes):` | `4` | 4 bytes per page/frame $\to$ 4 pages (0-3) and 8 frames (0-7) |
| `Frame number for page 0 (-1 if not in memory):` | `2` | Map Page 0 to Frame 2 |
| `Frame number for page 1 (-1 if not in memory):` | `0` | Map Page 1 to Frame 0 |
| `Frame number for page 2 (-1 if not in memory):` | `-1` | Page 2 is not loaded in RAM (tests **Page Fault**) |
| `Frame number for page 3 (-1 if not in memory):` | `5` | Map Page 3 to Frame 5 |
| `Enter base address:` | `1000` | Starting physical base address of memory |
| `Enter logical address (-1 to stop):` | `5` | Page 1, Offset 1 $\to$ Translated to PA **1001** |
| `Enter logical address (-1 to stop):` | `9` | Page 2, Offset 1 $\to$ Triggers **Page Fault** (not in memory) |
| `Enter logical address (-1 to stop):` | `-1` | Stops the program |

> **Quick Copy-Paste Input for Q1:**
> ```text
> 16
> 32
> 4
> 2
> 0
> -1
> 5
> 1000
> 5
> 9
> -1
> ```

---

### 🖥️ Expected Output Walkthrough (Q1)

```text
Enter logical memory size (bytes): 16
Enter physical memory size (bytes): 32
Enter page size (bytes): 4

Number of pages = 4
Number of frames = 8

Frame number for page 0 (-1 if not in memory): 2
Frame number for page 1 (-1 if not in memory): 0
Frame number for page 2 (-1 if not in memory): -1
Frame number for page 3 (-1 if not in memory): 5

PAGE TABLE
Page    Page Address   Frame Number  Presence Bit
0       0              2             1
1       4              0             1
2       8              -             0
3       12             5             1

FRAME TABLE
Frame   Frame Address   Page Number
0       0               1
1       4               -
2       8               0
3       12              -
4       16              -
5       20              3
6       24              -
7       28              -

Enter base address: 1000

Enter logical address (-1 to stop): 5
Page number = 1, Offset = 1
Frame number = 0
Physical address = 1000 + 0 * 4 + 1 = 1001

Enter logical address (-1 to stop): 9
Page number = 2, Offset = 1
Page 2 is not in memory: Page Fault

Enter logical address (-1 to stop): -1
```

---

## 📗 Question 2: Two-Level Hierarchical Paging (`q2.cpp`)

### Overview
Virtual address is split into 3 fields:
$$\text{Virtual Address} = [\text{PDI: Page Directory Index}] \mid [\text{PTI: Page Table Index}] \mid [\text{Offset}]$$

- $\text{Offset Bits} = \text{Total Bits} - \text{PDI Bits} - \text{PTI Bits}$
- $\text{Page Size} = 2^{\text{Offset Bits}}$ bytes
- $\text{Number of Page Directory Entries} = 2^{\text{PDI Bits}}$
- $\text{Number of Page Table Entries per Table} = 2^{\text{PTI Bits}}$
- $\text{Number of Frames} = \text{Physical Memory Size} / \text{Page Size}$
- Translates logical address using two lookups:
  1. Check **Page Directory** at `PDI`. If not present $\to$ **Page Fault**.
  2. Check **Page Table** at `PTI`. If not present $\to$ **Page Fault**.
  3. If present $\to \text{Physical Address} = (\text{Frame Number} \times \text{Page Size}) + \text{Offset}$.

---

### ⌨️ Inputs to Enter While Running (Q2)

When prompted, enter the following values step by step:

| Prompt | What to Enter | Reason / Explanation |
| :--- | :---: | :--- |
| `Enter virtual address size (bits):` | `12` | 12-bit virtual address ($2^{12} = 4096$ bytes) |
| `Enter number of PDI bits:` | `2` | 2 bits $\to 2^2 = 4$ Page Directory entries (PDI 0 to 3) |
| `Enter number of PTI bits:` | `2` | 2 bits $\to 2^2 = 4$ entries per Page Table (PTI 0 to 3) |
| `Enter physical memory size (bytes):` | `2048` | Physical memory size |

*(Calculated: Offset bits = $12 - 2 - 2 = 8$ bits, Page size = $2^8 = 256$ bytes, Frames = $2048 / 256 = 8$ frames).*

| Mapping Prompt | What to Enter | Explanation |
| :--- | :---: | :--- |
| `How many pages do you want to map?` | `3` | Map 3 virtual pages |
| `Enter PDI, PTI and frame number:` | `0 0 2` | Map PDI 0, PTI 0 to Frame 2 |
| `Enter PDI, PTI and frame number:` | `0 1 5` | Map PDI 0, PTI 1 to Frame 5 |
| `Enter PDI, PTI and frame number:` | `1 0 0` | Map PDI 1, PTI 0 to Frame 0 |

| Address Translation Prompt | What to Enter | Binary / Breakdown | Result |
| :--- | :---: | :--- | :--- |
| `Enter logical address (-1 to stop):` | `266` | $\text{PDI}=0, \text{PTI}=1, \text{Offset}=10$ | **Valid translation**: Frame 5 $\to \text{PA} = 5 \times 256 + 10 = \mathbf{1290}$ |
| `Enter logical address (-1 to stop):` | `517` | $\text{PDI}=0, \text{PTI}=2, \text{Offset}=5$ | **Page Fault** (Page Table PT0 entry 2 is empty) |
| `Enter logical address (-1 to stop):` | `2048` | $\text{PDI}=2, \text{PTI}=0, \text{Offset}=0$ | **Page Fault** (Page Directory entry 2 not present) |
| `Enter logical address (-1 to stop):` | `-1` | — | Stops the program |

> **Quick Copy-Paste Input for Q2:**
> ```text
> 12
> 2
> 2
> 2048
> 3
> 0 0 2
> 0 1 5
> 1 0 0
> 266
> 517
> 2048
> -1
> ```

---

### 🖥️ Expected Output Walkthrough (Q2)

```text
Enter virtual address size (bits): 12
Enter number of PDI bits: 2
Enter number of PTI bits: 2
Enter physical memory size (bytes): 2048

Offset bits = 8
Page size = 256 bytes
Number of virtual pages = 16
Number of frames = 8
Page directory = 4 entries, 16 bytes
Each page table = 4 entries, 16 bytes

How many pages do you want to map? 3
Enter PDI, PTI and frame number: 0 0 2
Enter PDI, PTI and frame number: 0 1 5
Enter PDI, PTI and frame number: 1 0 0

PAGE DIRECTORY
PDI   Page Table    Presence Bit
0     PT0           1
1     PT1           1
2     -             0
3     -             0

PAGE TABLE PT0
PTI   Frame Number  Presence Bit
0     2             1
1     5             1
2     -             0
3     -             0

PAGE TABLE PT1
PTI   Frame Number  Presence Bit
0     0             1
1     -             0
2     -             0
3     -             0

Page tables allocated = 2 of 4 (32 bytes)

Enter logical address (-1 to stop): 266
PDI = 0, PTI = 1, Offset = 10
Frame number = 5
Physical address = 5 * 256 + 10 = 1290

Enter logical address (-1 to stop): 517
PDI = 0, PTI = 2, Offset = 5
Page not present in PT0: Page Fault

Enter logical address (-1 to stop): 2048
PDI = 2, PTI = 0, Offset = 0
Page directory entry 2 not present: Page Fault

Enter logical address (-1 to stop): -1
```
