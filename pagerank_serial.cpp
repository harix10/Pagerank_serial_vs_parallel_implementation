#include <iostream>
#include <vector>
#include <iomanip>
#include <omp.h>
#include <cmath>
#include <numeric>

using namespace std;

// ============================================================
// PageRank Parameters
// ============================================================

const double DAMPING_FACTOR = 0.85;
const int ITERATIONS = 50;

// ============================================================
// Graph Structure
// ============================================================

struct Graph
{
    int numNodes;

    vector<vector<int>> outgoing;
    vector<vector<int>> incoming;
    vector<int> outDegree;

    Graph(int n)
    {
        numNodes = n;
        outgoing.resize(n);
        incoming.resize(n);
        outDegree.resize(n, 0);
    }

    void addEdge(int u, int v)
    {
        outgoing[u].push_back(v);
        incoming[v].push_back(u);
        outDegree[u]++;
    }
};

// ============================================================
// Serial PageRank Algorithm
// ============================================================

vector<double> pageRankSerial(
    const Graph &graph,
    int iterations,
    double damping)
{
    int N = graph.numNodes;

    // Initial PageRank: equal probability for every node
    vector<double> current(N, 1.0 / N);
    vector<double> next(N, 0.0);

    double base = (1.0 - damping) / N;

    for (int iter = 0; iter < iterations; iter++)
    {
        // ----------------------------------------------------
        // Step 1: Calculate total PageRank of dangling nodes
        // ----------------------------------------------------

        double danglingSum = 0.0;

        for (int node = 0; node < N; node++)
        {
            if (graph.outDegree[node] == 0)
            {
                danglingSum += current[node];
            }
        }

        double danglingShare =
            damping * danglingSum / N;

        // ----------------------------------------------------
        // Step 2: Calculate PageRank of every node
        // ----------------------------------------------------

        for (int node = 0; node < N; node++)
        {
            double incomingSum = 0.0;

            // Process all nodes pointing to 'node'
            for (int source : graph.incoming[node])
            {
                if (graph.outDegree[source] > 0)
                {
                    incomingSum +=
                        current[source] /
                        graph.outDegree[source];
                }
            }

            next[node] =
                base +
                danglingShare +
                damping * incomingSum;
        }

        // Prepare for next iteration
        current.swap(next);
    }

    return current;
}

// ============================================================
// Utility Functions
// ============================================================

// Calculate sum of all PageRank values
double calculateChecksum(const vector<double> &rank)
{
    return accumulate(rank.begin(), rank.end(), 0.0);
}

// Display PageRank values
void displayPageRank(const vector<double> &rank)
{
    cout << fixed << setprecision(9);

    for (size_t i = 0; i < rank.size(); i++)
    {
        cout << "Node " << i<< " : " << rank[i] << '\n';
    }
}

// Check whether PageRank values sum approximately to 1
bool checkCorrectness(const vector<double> &rank)
{
    double sum = calculateChecksum(rank);

    return fabs(sum - 1.0) < 1e-9;
}

// ============================================================
// Test Case 1
// Three-node cycle
//
// 0 -> 1
// 1 -> 2
// 2 -> 0
// ============================================================

Graph createTestCase1()
{
    Graph graph(3);

    graph.addEdge(0, 1);
    graph.addEdge(1, 2);
    graph.addEdge(2, 0);

    return graph;
}

// ============================================================
// Test Case 2
// Four-node graph with a dangling node
//
// 0 -> 1
// 0 -> 2
// 1 -> 2
// 2 -> 3
// 3 -> (no outgoing edge)
// ============================================================

Graph createTestCase2()
{
    Graph graph(4);

    graph.addEdge(0, 1);
    graph.addEdge(0, 2);
    graph.addEdge(1, 2);
    graph.addEdge(2, 3);

    return graph;
}

// ============================================================
// Test Case 3
// Five-node graph
//
// 0 -> 1, 2
// 1 -> 2
// 2 -> 0, 3
// 3 -> 4
// 4 -> 2
// ============================================================

Graph createTestCase3()
{
    Graph graph(5);

    graph.addEdge(0, 1);
    graph.addEdge(0, 2);

    graph.addEdge(1, 2);

    graph.addEdge(2, 0);
    graph.addEdge(2, 3);

    graph.addEdge(3, 4);

    graph.addEdge(4, 2);

    return graph;
}

// ============================================================
// Run a Correctness Test Case
// ============================================================

void runTestCase(
    const string &testName,
    const Graph &graph)
{
    cout << "\n==================================================\n";
    cout << testName << '\n';
    cout << "Number of Nodes : "<< graph.numNodes << '\n';

    int edges = 0;

    for (int degree : graph.outDegree)
    {
        edges += degree;
    }

    cout << "Number of Edges : "<< edges << '\n';

    cout << "--------------------------------------------------\n";

    vector<double> rank =
        pageRankSerial(
            graph,
            ITERATIONS,
            DAMPING_FACTOR);

    displayPageRank(rank);

    double checksum =
        calculateChecksum(rank);

    cout << "--------------------------------------------------\n";

    cout << fixed << setprecision(12);

    cout << "PageRank Checksum : "<< checksum << '\n';

    if (checkCorrectness(rank))
    {
        cout << "Correctness       : PASS\n";
    }
    else
    {
        cout << "Correctness       : FAIL\n";
    }
}

// ============================================================
// Generate Large Graph
// Each node has 'edgesPerNode' outgoing edges.
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

            if (u != v)
            {
                graph.addEdge(u, v);
            }
        }
    }

    return graph;
}

// ============================================================
// Performance Test
// ============================================================

void runPerformanceTest(
    int numNodes,
    int edgesPerNode)
{
    cout << "\n==================================================\n";

    cout << "Nodes           : "<< numNodes << '\n';
    cout << "Edges per Node  : "<< edgesPerNode << '\n';

    Graph graph =
        generateLargeGraph(
            numNodes,
            edgesPerNode);

    long long totalEdges = 0;

    for (int degree : graph.outDegree)
    {
        totalEdges += degree;
    }

    cout << "Total Edges     : "<< totalEdges << '\n';

    cout << "Iterations      : "<< ITERATIONS << '\n';

    cout << "--------------------------------------------------\n";

    // Start timer
    double start = omp_get_wtime();

    // Run PageRank
    vector<double> rank =
        pageRankSerial(
            graph,
            ITERATIONS,
            DAMPING_FACTOR);

    // Stop timer
    double end = omp_get_wtime();

    double elapsed = (end - start) * 1000.0;

    double checksum =
        calculateChecksum(rank);

    cout << fixed << setprecision(6);

    cout << "Serial Time (ms): "<< elapsed << '\n';

    cout << setprecision(12);

    cout << "Checksum        : "<< checksum << '\n';

    if (checkCorrectness(rank))
    {
        cout << "Correctness     : PASS\n";
    }
    else
    {
        cout << "Correctness     : FAIL\n";
    }
}

// ============================================================
// Main Function
// ============================================================

int main()
{
    cout << "==================================================\n";
    cout << "       PARALLEL AND DISTRIBUTED COMPUTING\n";
    cout << "             SERIAL PAGERANK\n";
    cout << "==================================================\n";

    cout<< "Team Members:"<< endl;
    cout << "Student Name : "<< "Hari Krishnan S" << endl;
    cout << "Roll No      : "<< "2024bcs0116" << endl;

    cout << "\nStudent Name : "<< "Shivkarthik Suresh" << endl;
    cout << "Roll No      : "<< "2024bcs0092" << endl;

    cout << "==================================================\n";

    cout << "\nPageRank Parameters\n";
    cout << "Damping Factor : "<< DAMPING_FACTOR << '\n';

    cout << "Iterations     : "<< ITERATIONS << '\n';

    // ========================================================
    // Three Required Test Cases
    // ========================================================

    cout << "\n\n******** CORRECTNESS TEST CASES ********\n";

    Graph test1 = createTestCase1();
    runTestCase(
        "TEST CASE 1 - Three Node Cycle",
        test1);

    Graph test2 = createTestCase2();
    runTestCase(
        "TEST CASE 2 - Graph with Dangling Node",
        test2);

    Graph test3 = createTestCase3();
    runTestCase(
        "TEST CASE 3 - Five Node Graph",
        test3);

    // ========================================================
    // Increasing Input Size
    // ========================================================

    cout << "\n\n******** PERFORMANCE TESTS ********\n";

    runPerformanceTest(1000, 5);
    runPerformanceTest(5000, 5);
    runPerformanceTest(10000, 5);
    runPerformanceTest(50000, 5);

    return 0;
}