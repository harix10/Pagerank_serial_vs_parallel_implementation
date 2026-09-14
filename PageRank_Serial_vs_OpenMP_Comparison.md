# Comparison of Serial and OpenMP PageRank Results

## 1. Overview

This document compares the final **Serial PageRank** and **OpenMP Parallel PageRank** results using the actual outputs obtained from both programs.

Both implementations use:

- Damping factor: **0.85**
- Number of iterations: **50**
- Three correctness test cases
- Large graphs with **5 edges per node**
- Input sizes of **1,000, 5,000, 10,000, and 50,000 nodes**

The OpenMP implementation was additionally tested with **1, 2, 4, and 8 threads**.

---

## 2. Correctness Comparison

The PageRank values produced by the serial and parallel implementations are the same to the displayed precision for all three test cases.

### Test Case 1 – Three Node Cycle

Graph:

```text
0 -> 1
1 -> 2
2 -> 0
```

| Node | Serial | OpenMP |
|---|---:|---:|
| 0 | 0.333333333 | 0.333333 |
| 1 | 0.333333333 | 0.333333 |
| 2 | 0.333333333 | 0.333333 |

Checksum:

```text
Serial:  1.000000000000
OpenMP:  1.000000
```

**Result: PASS**

---

### Test Case 2 – Graph with Dangling Node

Graph:

```text
0 -> 1
0 -> 2
1 -> 2
2 -> 3
3 -> no outgoing edge
```

| Node | Serial | OpenMP |
|---|---:|---:|
| 0 | 0.120451996 | 0.120452 |
| 1 | 0.171644094 | 0.171644 |
| 2 | 0.317541575 | 0.317542 |
| 3 | 0.390362335 | 0.390362 |

Checksum:

```text
Serial:  1.000000000000
OpenMP:  1.000000
```

**Result: PASS**

---

### Test Case 3 – Five Node Graph

Graph:

```text
0 -> 1, 2
1 -> 2
2 -> 0, 3
3 -> 4
4 -> 2
```

| Node | Serial | OpenMP |
|---|---:|---:|
| 0 | 0.179566224 | 0.179566 |
| 1 | 0.106315651 | 0.106316 |
| 2 | 0.351920600 | 0.351921 |
| 3 | 0.179566224 | 0.179566 |
| 4 | 0.182631302 | 0.182631 |

Checksum:

```text
Serial:  1.000000000000
OpenMP:  1.000000
```

**Result: PASS**

### Correctness conclusion

The PageRank results of the serial and OpenMP implementations agree to the displayed precision for all three test cases. The checksum is approximately **1.0** in every case, confirming that the PageRank values are correctly normalized.

---

## 3. Execution Time Comparison – 4 Threads

For the following comparison, the OpenMP program was executed using **4 threads**.

| Nodes | Edges | Serial Time (ms) | OpenMP Time (ms) | Time Difference (ms) | OpenMP Speedup |
|---:|---:|---:|---:|---:|---:|
| 1,000 | 5,000 | 5.000114 | 9.0001 | -3.999986 | 0.56× |
| 5,000 | 25,000 | 30.999899 | 19.0001 | 11.999799 | 1.63× |
| 10,000 | 50,000 | 60.999870 | 32.0001 | 28.999770 | 1.91× |
| 50,000 | 250,000 | 429.000139 | 115.9999 | 313.000239 | 3.70× |

Speedup is calculated as:

```text
Speedup = Serial Time / OpenMP Time
```

### Observations

- For **1,000 nodes**, the OpenMP version is slower than the serial version. This is expected for a relatively small input because OpenMP thread-management and synchronization overhead can dominate the actual computation.
- From **5,000 nodes onward**, OpenMP performs faster than the serial implementation.
- For **50,000 nodes**, the OpenMP version reduces execution time from approximately **429 ms to 116 ms**.
- The corresponding speedup is approximately:

```text
429.000139 / 115.9999 = 3.70×
```

This demonstrates that parallel processing becomes more beneficial as the input size increases.

---

## 4. OpenMP Thread Performance for 50,000 Nodes

The OpenMP program was also tested with different numbers of threads for the same graph containing 50,000 nodes and 250,000 edges.

| Threads | Time (ms) | Speedup vs 1 Thread | Parallel Efficiency |
|---:|---:|---:|---:|
| 1 | 437.0000 | 1.00× | 100.00% |
| 2 | 214.9999 | 2.03× | 101.63% |
| 4 | 128.9999 | 3.39× | 84.76% |
| 8 | 70.0002 | 6.24× | 78.03% |

Speedup is calculated using the 1-thread execution time:

```text
Speedup = T(1 thread) / T(P threads)
```

For 8 threads:

```text
Speedup = 437.0000 / 70.0002
        ≈ 6.24×
```

Parallel efficiency is:

```text
Efficiency = Speedup / Number of Threads × 100
```

For 8 threads:

```text
Efficiency = 6.24 / 8 × 100
           ≈ 78.03%
```

### Observation

Increasing the number of OpenMP threads generally decreases the execution time:

```text
1 thread  -> 437.0000 ms
2 threads -> 214.9999 ms
4 threads -> 128.9999 ms
8 threads -> 70.0002 ms
```

The best measured execution time is obtained with **8 threads**, at approximately **70 ms**.

The speedup is not perfectly linear because of OpenMP overhead, synchronization, reduction operations, memory access limitations, and portions of the algorithm that cannot be parallelized.

---

## 5. Scaling with Input Size

For the large-graph tests, every node has 5 outgoing edges. Therefore:

```text
Edges = 5 × Nodes
```

The measured serial execution time increases as the graph size increases:

| Nodes | Edges | Serial Time (ms) |
|---:|---:|---:|
| 1,000 | 5,000 | 5.000114 |
| 5,000 | 25,000 | 30.999899 |
| 10,000 | 50,000 | 60.999870 |
| 50,000 | 250,000 | 429.000139 |

The OpenMP execution time with 4 threads was:

| Nodes | Edges | OpenMP Time (ms) |
|---:|---:|---:|
| 1,000 | 5,000 | 9.0001 |
| 5,000 | 25,000 | 19.0001 |
| 10,000 | 50,000 | 32.0001 |
| 50,000 | 250,000 | 115.9999 |

The results show that the benefit of parallelization becomes more significant for larger input sizes.

---

## 6. Complexity Comparison

Let:

- `N` = number of nodes
- `E` = number of edges
- `I` = number of PageRank iterations
- `P` = number of OpenMP threads

### Serial implementation

Each iteration processes the nodes and incoming edges:

```text
T_serial = O(I(N + E))
```

Since the program uses a fixed 50 iterations:

```text
T_serial = O(N + E)
```

For the generated graphs, `E = 5N`, so:

```text
T_serial = O(N)
```

### OpenMP implementation

The major loops are distributed among threads. Ideally:

```text
T_parallel ≈ O(I(N + E) / P)
```

plus parallel overhead such as synchronization, thread scheduling and reduction.

For fixed 50 iterations and fixed edge density:

```text
T_parallel ≈ O(N / P)
```

in the idealized parallel-work model.

---

## 7. Overall Comparison

| Aspect | Serial PageRank | OpenMP PageRank |
|---|---|---|
| Execution model | Sequential | Parallel |
| PageRank correctness | PASS | PASS |
| Checksum | Approximately 1.0 | Approximately 1.0 |
| Small input performance | Better in the 1,000-node test | Overhead makes it slower |
| Large input performance | Slower | Faster |
| Multiple threads | Not applicable | Supported |
| Best measured time for 50,000 nodes | 429.000139 ms | 70.0002 ms with 8 threads |
| Main advantage | Simple execution | Reduced execution time for larger graphs |

---

## 8. Final Conclusion

The experimental results show that both the serial and OpenMP PageRank implementations produce equivalent PageRank results for all three correctness test cases. The checksum remains approximately **1.0**, confirming the correctness of the calculations.

For small input sizes, the OpenMP version can be slower because the computation is too small to compensate for parallelization overhead. However, as the graph size increases, OpenMP provides a clear performance advantage.

For the 50,000-node graph, the 4-thread OpenMP implementation reduced the measured execution time from **429.000139 ms** in the serial implementation to **115.9999 ms**, giving approximately **3.70× speedup**.

When different thread counts were tested on the 50,000-node graph, the best measured result was **70.0002 ms with 8 threads**, corresponding to approximately **6.24× speedup compared with the 1-thread OpenMP execution**.

Therefore, the experiment demonstrates that OpenMP parallelization is effective for computationally larger PageRank problems, while the benefit is less noticeable for small inputs due to parallel execution overhead.
