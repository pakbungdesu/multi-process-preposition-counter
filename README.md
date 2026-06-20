# Multi-Process Preposition Counter

A high-performance C++ tool that leverages POSIX multiprocessing (`fork()`) and Inter-Process Communication (IPC Shared Memory) to concurrently analyze text files and count occurrences of a dynamic list of target prepositions.

---

## Features

* **Parallel Processing:** Spawns multiple concurrent child processes via `fork()` to process separate text partitions simultaneously.
* **IPC Shared Memory:** Utilizes System V shared memory segment allocations (`shmget`, `shmat`) to securely bubble up structured structural metrics from children to parent.
* **Text Normalization:** Sanitizes input pipelines automatically by parsing clean token blocks, stripping punctuation, and unifying character casing.
* **Timestamped Reporting:** Automatically aggregates calculations and exports structured output cleanly to a unique, timestamp-hashed `.csv` log.

---

## Architecture Overview

```
               [ Parent Process ]
             /         |        \
       (fork)       (fork)       (fork)
         /             |             \
  [Child 1]        [Child 2]        [Child 3]
 (lotr1.txt)      (lotr2.txt)      (lotr3.txt)
         \             |             /
          v            v            v
     +------------------------------------+
     |    System V Shared Memory Block    |
     +------------------------------------+
                       |
                       v (Consolidate & Aggregate)
               [ Final CSV Output ]

```

---

## File Requirements

The program expects the following configuration and raw content pieces present inside your target working directory:

1. **`prepositions.txt`**: A plaintext file containing target prepositions to index, separated by whitespace or line breaks.
2. **Target Corpus Files**: Exactly three raw text resource blocks mapped matching the naming convention:
* `lotr1.txt`
* `lotr2.txt`
* `lotr3.txt`



---

## Getting Started

### Prerequisites

This code targets POSIX-compliant environments (Linux, macOS, WSL) due to explicit architecture bindings including `<sys/shm.h>`, `<sys/wait.h>`, and `unistd.h`.

### Compilation

Compile the source using any modern C++ compiler (`g++` or `clang++` supporting C++11 or higher):

```bash
g++ -std=c++11 main.cpp -o prep_counter

```

### Running the Application

1. Ensure all input asset files (`prepositions.txt`, `lotr1.txt`, etc.) populate the execution target space.
2. Execute the compiled binary build artifact:

```bash
./prep_counter

```

---

## Output Generation

Upon processing completion, individual analytical results are reduced down to a pipeline summary inside the standard output console interface before spawning an external payload write:

```text
Final counts across all files:
of | 4312
to | 2911
in | 1845

Results saved to output_2026_06_20_16_55_18.csv

```

The output file uses a standard delimiter layout format:

```csv
Preposition|Count
of|4312
to|2911
in|1845

```