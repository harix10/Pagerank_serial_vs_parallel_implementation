#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <numeric>
#include <omp.h>

using namespace std;

// ============================================================
// PAGE RANK PARAMETERS
// ============================================================
const double DAMPING_FACTOR = 0.85;
const int ITERATIONS = 50;

// ============================================================
// GRAPH STRUCTURE
// ============================================================
struct Graph
{
    int numNodes;

    // outgoing[u] contains nodes to which u has outgoing edges
    vector<vector<int>> outgoing;

    // incoming[v] contains nodes that point to v
    vector<vector<int>> incoming;

    // outDegree[u] = number of outgoing edges from u
    vector<int> outDegree;

    Graph(int n)
    {
        numNodes = n;
        outgoing.resize(n);
        incoming.resize(n);
        outDegree.resize(n, 0);
    }

    // Add directed edge u -> v
    void addEdge(int u, int v)
    {
        outgoing[u].push_back(v);
        incoming[v].push_back(u);
        outDegree[u]++;
    }
};

// ============================================================
// PARALLEL PAGERANK USING OPENMP
// ============================================================
vector<double> pageRankOpenMP(
    const Graph& graph,
    int iterations,
    double damping)
{
    int N = graph.numNodes;

    // Initial PageRank: 1/N for every node
    vector<double> current(N, 1.0 / N);
    vector<double> next(N, 0.0);

    // Teleportation/base contribution
    double base = (1.0 - damping) / N;

    // Repeat PageRank calculation
    for (int iteration = 0; iteration < iterations; iteration++)
    {
        // ----------------------------------------------------
        // Calculate total PageRank from dangling nodes
        // ----------------------------------------------------
        double danglingSum = 0.0;

        #pragma omp parallel for reduction(+ : danglingSum)
        for (int node = 0; node < N; node++)
        {
            if (graph.outDegree[node] == 0)
            {
                danglingSum += current[node];
            }
        }

        // Distribute dangling-node PageRank equally
        double danglingShare = damping * danglingSum / N;

        // ----------------------------------------------------
        // Calculate new PageRank for every node
        // ----------------------------------------------------
        #pragma omp parallel for schedule(static)
        for (int node = 0; node < N; node++)
        {
            double incomingSum = 0.0;

            // Calculate contribution from incoming neighbors
            for (int source : graph.incoming[node])
            {
                incomingSum +=
                    current[source] / graph.outDegree[source];
            }

            next[node] =
                base +
                danglingShare +
                damping * incomingSum;
        }

        // Move new values into current
        current.swap(next);
    }

    return current;
}

// ============================================================
// DISPLAY PAGERANK VALUES
// ============================================================
void displayPageRank(const vector<double>& rank)
{
    cout << fixed << setprecision(6);

    for (int i = 0; i < (int)rank.size(); i++)
    {
        cout << "Node " << i<< " : " << rank[i] << endl;
    }

    double sum = accumulate(rank.begin(), rank.end(), 0.0);

    cout << "Sum of PageRank = "<< sum << endl;
}

// ============================================================
// CHECK WHETHER PAGERANK SUM IS VALID
// ============================================================
bool isCorrect(const vector<double>& rank)
{
    double sum = accumulate(rank.begin(), rank.end(), 0.0);

    return fabs(sum - 1.0) < 1e-9;
}

// ============================================================
// TEST CASE 1
// 3-node cycle
//
// 0 -> 1
// 1 -> 2
// 2 -> 0
// ============================================================
void testCase1()
{
    cout << "\n========================================\n";
    cout << "TEST CASE 1: 3-Node Cycle\n";
    cout << "========================================\n";

    Graph graph(3);

    graph.addEdge(0, 1);
    graph.addEdge(1, 2);
    graph.addEdge(2, 0);

    vector<double> rank =
        pageRankOpenMP(
            graph,
            ITERATIONS,
            DAMPING_FACTOR);

    displayPageRank(rank);

    cout << "Correctness: "<< (isCorrect(rank) ? "PASS" : "FAIL")<< endl;
}

// ============================================================
// TEST CASE 2
//
// 0 -> 1
// 0 -> 2
// 1 -> 2
// 2 -> 3
// 3 -> nothing
// ============================================================
void testCase2()
{
    cout << "\n========================================\n";
    cout << "TEST CASE 2: Graph with Dangling Node\n";
    cout << "========================================\n";

    Graph graph(4);

    graph.addEdge(0, 1);
    graph.addEdge(0, 2);
    graph.addEdge(1, 2);
    graph.addEdge(2, 3);

    vector<double> rank =
        pageRankOpenMP(
            graph,
            ITERATIONS,
            DAMPING_FACTOR);

    displayPageRank(rank);

    cout << "Correctness: "<< (isCorrect(rank) ? "PASS" : "FAIL")<< endl;
}

// ============================================================
// TEST CASE 3
//
// 0 -> 1
// 0 -> 2
// 1 -> 2
// 2 -> 0
// 2 -> 3
// 3 -> 4
// 4 -> 2
// ============================================================
void testCase3()
{
    cout << "\n========================================\n";
    cout << "TEST CASE 3: 5-Node Graph\n";
    cout << "========================================\n";

    Graph graph(5);

    graph.addEdge(0, 1);
    graph.addEdge(0, 2);
    graph.addEdge(1, 2);
    graph.addEdge(2, 0);
    graph.addEdge(2, 3);
    graph.addEdge(3, 4);
    graph.addEdge(4, 2);

    vector<double> rank =
        pageRankOpenMP(
            graph,
            ITERATIONS,
            DAMPING_FACTOR);

    displayPageRank(rank);

    cout << "Correctness: "<< (isCorrect(rank) ? "PASS" : "FAIL")<< endl;
}

// ============================================================
// GENERATE LARGE GRAPH
// ============================================================
Graph generateLargeGraph(
    int numNodes,
    int edgesPerNode)
{
    Graph graph(numNodes);

    for (int u = 0; u < numNodes; u++)
    {
        for (int k = 1; k <= edgesPerNode; k++)
        {
            int v = (u + k) % numNodes;

            // Avoid self-loop
            if (v != u)
            {
                graph.addEdge(u, v);
            }
        }
    }

    return graph;
}

// ============================================================
// PERFORMANCE TEST
// ============================================================
void runPerformanceTest(
    int numNodes,
    int edgesPerNode,
    int numberOfThreads)
{
    Graph graph =
        generateLargeGraph(
            numNodes,
            edgesPerNode);

    // Set number of OpenMP threads
    omp_set_num_threads(numberOfThreads);

    // Start timer
    double start = omp_get_wtime();

    vector<double> rank =
        pageRankOpenMP(
            graph,
            ITERATIONS,
            DAMPING_FACTOR);

    // Stop timer
    double end = omp_get_wtime();

    // Convert seconds to milliseconds
    double elapsedMilliseconds =
        (end - start) * 1000.0;

    double checksum =
        accumulate(
            rank.begin(),
            rank.end(),
            0.0);

    cout << left << setw(12) << numNodes << setw(15) << edgesPerNode<< setw(12) << numberOfThreads<< setw(15) << fixed<< setprecision(4)<< elapsedMilliseconds<< setw(15)<< setprecision(6)<< checksum<< endl;
}

// ============================================================
// THREAD COMPARISON
// ============================================================
void compareThreads(
    int numNodes,
    int edgesPerNode)
{
    cout << "\n========================================\n";
    cout << "THREAD PERFORMANCE COMPARISON\n";
    cout << "========================================\n";

    cout << left<< setw(12) << "Nodes"<< setw(15) << "Edges/Node"<< setw(12) << "Threads"<< setw(15) << "Time(ms)"<< setw(15) << "Checksum"<< endl;

    cout << "------------------------------------------------------------\n";

    int threadCounts[] = {1, 2, 4, 8};

    for (int threads : threadCounts)
    {
        runPerformanceTest(
            numNodes,
            edgesPerNode,
            threads);
    }
}

// ============================================================
// MAIN FUNCTION
// ============================================================
int main()
{
    cout << "====================================================\n";
    cout << " PERFORMANCE ANALYSIS OF OPENMP PAGERANK\n";
    cout << "====================================================\n";

    cout<< "Team Members:"<< endl;
    cout << "Student Name : "<< "Hari Krishnan S" << endl;
    cout << "Roll No      : "<< "2024bcs0116" << endl;

    cout << "\nStudent Name : "<< "Shivkarthik Suresh" << endl;
    cout << "Roll No      : "<< "2024bcs0092" << endl;

    cout << "\nDamping Factor : "<< DAMPING_FACTOR << endl;

    cout << "Iterations     : "<< ITERATIONS << endl;

    cout << "Maximum OpenMP Threads Available : "<< omp_get_max_threads()<< endl;

    // --------------------------------------------------------
    // SMALL TEST CASES
    // --------------------------------------------------------
    testCase1();
    testCase2();
    testCase3();

    // --------------------------------------------------------
    // PERFORMANCE TESTS
    // --------------------------------------------------------
    cout << "\n========================================\n";
    cout << "PERFORMANCE TESTS - 4 THREADS\n";
    cout << "========================================\n";

    cout << left<< setw(12) << "Nodes"<< setw(15) << "Edges/Node"<< setw(12) << "Threads"<< setw(15) << "Time(ms)"<< setw(15) << "Checksum"<< endl;

    cout << "------------------------------------------------------------\n";

    runPerformanceTest(1000, 5, 4);
    runPerformanceTest(5000, 5, 4);
    runPerformanceTest(10000, 5, 4);
    runPerformanceTest(50000, 5, 4);

    // --------------------------------------------------------
    // COMPARE DIFFERENT THREAD COUNTS
    // --------------------------------------------------------
    compareThreads(50000, 5);

    cout << "\nProgram completed successfully.\n";

    return 0;
}