# Performance Analysis of Serial and OpenMP-Based PageRank

## Parallel and Distributed Computing (PDC)

**Batch:** 2024  
**Academic Year:** 2026  
**Assignment:** Assignment – I

### Team Members

- **Hari Krishnan S** — 2024BCS0116
- **Shivkarthik Suresh** — 2024BCS0092

---

## 1. Project Overview

This project implements and analyzes the **PageRank algorithm** using two approaches:

1. **Serial PageRank implementation in C++**
2. **Parallel PageRank implementation using OpenMP**

The purpose of the project is to study the performance difference between sequential and parallel execution for PageRank on directed graphs of different sizes.

The PageRank algorithm assigns a relative importance score to each node in a directed graph. The implementation uses a damping factor of **0.85** and performs **50 iterations**.

---

## 2. PageRank Algorithm

PageRank determines the importance of nodes based on the incoming links they receive from other nodes.

The PageRank equation used is:

PR(v) = (1-d)/N + (d × D)/N + d × Σ(PR(u) / L(u))

where:

- `PR(v)` = PageRank of node `v`
- `d` = damping factor
- `N` = number of nodes
- `D` = total PageRank of dangling nodes
- `u` = incoming neighbor of node `v`
- `L(u)` = number of outgoing links from node `u`

A **dangling node** is a node with no outgoing edges. Its PageRank is distributed equally among all nodes.

---

## 3. Graph Representation

The graph is represented using three main structures:

- `outgoing` – stores the nodes to which each node has outgoing edges.
- `incoming` – stores the nodes that point to each node.
- `outDegree` – stores the number of outgoing edges from each node.

This representation allows the PageRank calculation to efficiently process the incoming neighbors of every node.

---

## 4. Serial Implementation

The serial implementation calculates PageRank sequentially.

For every iteration:

1. Initialize the dangling-node PageRank sum.
2. Find all dangling nodes.
3. Calculate the PageRank contribution from dangling nodes.
4. Calculate the incoming contribution for every node.
5. Calculate the new PageRank value.
6. Swap the current and next PageRank arrays.
7. Repeat for 50 iterations.

The serial program also checks the PageRank checksum. The sum of all PageRank values should be approximately `1.0`.

---

## 5. OpenMP Parallel Implementation

The parallel version uses OpenMP to parallelize the computationally intensive loops.

### Parallel Block 1 – Dangling Node Sum

The dangling-node calculation uses:

```cpp
#pragma omp parallel for reduction(+ : danglingSum)
The `reduction` clause safely combines the partial sums calculated by different threads and prevents race conditions.

### Parallel Block 2 – PageRank Calculation

The calculation of the PageRank of individual nodes uses:

```cpp
#pragma omp parallel for schedule(static)
```

Each thread calculates PageRank for different nodes. The threads read from the common `current` array and write to different elements of the `next` array.

The arrays are swapped only after all threads finish calculating the current iteration.

---

## 6. Test Cases

Three correctness test cases are included.

### Test Case 1 – Three Node Cycle

```text
0 -> 1
1 -> 2
2 -> 0
```

There are 3 nodes and 3 edges.

The PageRank values converge to approximately:

```text
Node 0 : 0.333333
Node 1 : 0.333333
Node 2 : 0.333333
```

Checksum:

```text
1.000000
```

Result:

```text
PASS
```

---

### Test Case 2 – Graph with Dangling Node

```text
0 -> 1
0 -> 2
1 -> 2
2 -> 3
3 -> no outgoing edge
```

There are 4 nodes and 4 edges.

Node 3 is a dangling node.

The final PageRank values are approximately:

```text
Node 0 : 0.120452
Node 1 : 0.171644
Node 2 : 0.317542
Node 3 : 0.390362
```

Checksum:

```text
1.000000
```

Result:

```text
PASS
```

---

### Test Case 3 – Five Node Graph

```text
0 -> 1, 2
1 -> 2
2 -> 0, 3
3 -> 4
4 -> 2
```

There are 5 nodes and 7 edges.

The final PageRank values are approximately:

```text
Node 0 : 0.179566
Node 1 : 0.106316
Node 2 : 0.351921
Node 3 : 0.179566
Node 4 : 0.182631
```

Checksum:

```text
1.000000
```

Result:

```text
PASS
```

---

## 7. Input Size Performance Testing

For the large-graph performance tests, every node has 5 outgoing edges.

The following input sizes are tested:

```text
1,000 nodes
5,000 nodes
10,000 nodes
50,000 nodes
```

Therefore:

```text
Edges = 5 × Nodes
```

The program performs 50 PageRank iterations for each input size.

---

## 8. Serial Performance

The measured serial execution times are:

| Nodes | Edges | Serial Time (ms) |
|------:|------:|-----------------:|
| 1,000 | 5,000 | 5.000114 |
| 5,000 | 25,000 | 30.999899 |
| 10,000 | 50,000 | 60.999870 |
| 50,000 | 250,000 | 429.000139 |

The execution time generally increases as the graph size increases.

---

## 9. OpenMP Performance

The OpenMP implementation was tested using 4 threads.

| Nodes | Edges | OpenMP Time (ms) |
|------:|------:|-----------------:|
| 1,000 | 5,000 | 9.0001 |
| 5,000 | 25,000 | 19.0001 |
| 10,000 | 50,000 | 32.0001 |
| 50,000 | 250,000 | 115.9999 |

For small input sizes, OpenMP can be slower because thread-management and synchronization overhead can dominate the computation.

For larger inputs, OpenMP provides a significant performance advantage.

---

## 10. Thread Performance Comparison

For a graph containing 50,000 nodes and 250,000 edges, the OpenMP implementation was tested with different numbers of threads.

| Threads | Time (ms) | Speedup |
|--------:|----------:|--------:|
| 1 | 437.0000 | 1.00× |
| 2 | 214.9999 | 2.03× |
| 4 | 128.9999 | 3.39× |
| 8 | 70.0002 | 6.24× |

Speedup is calculated as:

```text
Speedup = T(1 thread) / T(P threads)
```

For 8 threads:

```text
Speedup = 437.0000 / 70.0002
        ≈ 6.24×
```

The best measured execution time was approximately **70 ms with 8 threads**.

---

## 11. Complexity Analysis

Let:

- `N` = number of nodes
- `E` = number of edges
- `I` = number of PageRank iterations
- `P` = number of OpenMP threads

### Serial Complexity

Each iteration processes the nodes and incoming edges:

```text
T_serial = O(I(N + E))
```

Since the program uses a fixed 50 iterations:

```text
T_serial = O(N + E)
```

For the generated graphs:

```text
E = 5N
```

Therefore:

```text
T_serial = O(N)
```

### OpenMP Complexity

The major computational loops are distributed among OpenMP threads.

Ideally:

```text
T_parallel ≈ O(I(N + E) / P)
```

There is additional overhead due to:

- Thread scheduling
- Synchronization
- Reduction operations
- Memory access limitations

For fixed 50 iterations and fixed edge density:

```text
T_parallel ≈ O(N / P)
```

in the idealized parallel-work model.

---

## 12. Timing Method

The implementation uses the OpenMP wall-clock timing function:

```cpp
omp_get_wtime()
```

The elapsed time is calculated as:

```cpp
double start = omp_get_wtime();

... PageRank computation ...

double end = omp_get_wtime();

double elapsed = (end - start) * 1000.0;
```

The result is multiplied by 1000 to convert seconds into milliseconds.

---

## 13. Correctness Verification

Both the serial and OpenMP implementations produce equivalent PageRank values to the displayed precision for all three test cases.

The PageRank checksum remains approximately:

```text
1.000000
```

This confirms that the PageRank values are correctly normalized.

---

## 14. Compilation

### Serial Program

The serial program uses `omp_get_wtime()` for timing, so it should be compiled with OpenMP support:

```bash
g++ -O2 -std=c++17 -fopenmp pagerank_serial.cpp -o pagerank_serial
```

Run:

```bash
./pagerank_serial
```

On Windows PowerShell:

```powershell
.\pagerank_serial.exe
```

### OpenMP Program

Compile using:

```bash
g++ -O2 -std=c++17 -fopenmp pagerank_parallel.cpp -o pagerank_parallel
```

Run:

```bash
./pagerank_parallel
```

On Windows PowerShell:

```powershell
.\pagerank_parallel.exe
```

---

## 15. Files Included

The project contains:

```text
pagerank_serial.cpp
pagerank_serial.exe

pagerank_parallel.cpp
pagerank_parallel.exe
```

The assignment documentation contains:

```text
Module I – Serial PageRank
Module II – OpenMP Parallel PageRank
Test cases and outputs
Performance analysis
Complexity analysis
Speedup analysis
Final comparison and conclusion
```

---

## 16. Conclusion

The project demonstrates the performance difference between serial and OpenMP-based PageRank implementations.

Both implementations produce correct PageRank results, with the checksum remaining approximately 1.0 for all test cases.

For small graphs, OpenMP may be slower because of parallelization overhead. However, as the graph size increases, parallel execution becomes more beneficial.

For the 50,000-node graph, the 4-thread OpenMP implementation reduced the measured execution time from approximately 429 ms in the serial implementation to approximately 116 ms, achieving about 3.70× speedup.

With 8 OpenMP threads, the best measured execution time was approximately 70 ms, corresponding to approximately 6.24× speedup compared with the 1-thread OpenMP execution.

Therefore, the experiment demonstrates that OpenMP parallelization can significantly improve the execution time of computationally larger PageRank problems.
