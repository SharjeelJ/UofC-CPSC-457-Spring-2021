#include "fatsim.h"
#include <algorithm>
#include <stack>

using namespace std;

// reimplement this function
std::vector<long> fat_check(const std::vector<long> &fat) {
    // Creates an adjacency list of the size of the passed in vector
    vector<vector<long>> adjacencyList(fat.size());

    // Creates a vector to store the chain length of each node (so stores the maximum chain length of all its sub-nodes) with a default value for -1 for each node (will help in indicating which nodes have been parsed)
    vector<long> nodesChainLength(fat.size(), -1);

    // Creates a vector consisting of all nodes leading to a terminating node
    vector<long> terminatingNodes;

    // Populates the adjacency list as well as the vector containing which nodes lead to a terminating node
    for (int node = 0; node < fat.size(); node++)
        // Checks to see if the current node leads to a terminating node and adds it to the appropriate vector/list
        if (fat[node] == -1)
            terminatingNodes.push_back(node);
        else
            adjacencyList[fat[node]].push_back(node);

    // Creates the vector that will store the length of all possible chains leading up to a terminating node (will be returned to the calling code once populated)
    vector<long> nodeChainsResult;

    // Creates a stack to store the nodes that need to still be parsed
    stack<long> nodesToParseStack;

    // Populates the stack with the known nodes that lead to terminating nodes
    for (long node : terminatingNodes)
        nodesToParseStack.push(node);

    // Checks to see if there is at least 1 node that leads to a terminating node otherwise skips running DFS
    if (!terminatingNodes.empty()) {
        // Stores the most recent node that leads to a terminating node encountered
        long currentTerminatingNode = terminatingNodes.size() - 1;

        // Pushes a default value of the longest chain being 1 to the result vector for the current node
        nodeChainsResult.push_back(1);

        // Performs DFS and populates the result vector
        while (!nodesToParseStack.empty()) {
            // Stores the current node that will be parsed and removes it from the stack
            long currentNode = nodesToParseStack.top();
            nodesToParseStack.pop();

            // Updates the current node's length (marks it as visited)
            if (nodesChainLength[currentNode] == -1) {
                nodesChainLength[currentNode] = 1;

                // Checks to see if there are any immediate sub-nodes of the current node
                if (adjacencyList[currentNode].size() > 0) {
                    // Populates the stack with all the immediate sub-nodes of the current node
                    for (long subNode : adjacencyList[currentNode]) {
                        // Only adds sub-nodes to the stack that have not previously been visited
                        if (nodesChainLength[subNode] == -1)
                            nodesToParseStack.push(subNode);
                    }
                }
            }
        }
    }

    // Sorts the longest chains possible vector to be in ascending order
    sort(nodeChainsResult.begin(), nodeChainsResult.end());

    // Returns the longest chains possible result back to the calling code
    return nodeChainsResult;
}
