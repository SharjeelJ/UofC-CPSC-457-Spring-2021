#include "fatsim.h"
#include <cstdio>
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

    // Creates the vector that will store the length of all possible chains leading up to a terminating node
    vector<long> nodeChainsResult;

    stack<long> nodesToParseStack;
    for (long node : terminatingNodes)
        nodesToParseStack.push(node);


    if (!terminatingNodes.empty()) {
        long currentTerminatingNode = terminatingNodes.size() - 1;
        nodeChainsResult.push_back(1);
        while (!nodesToParseStack.empty()) {
            long currentNode = nodesToParseStack.top();
            nodesToParseStack.pop();

            if (nodesChainLength[currentNode] == -1)
                nodesChainLength[currentNode] = 1;

            if (adjacencyList[currentNode].size() > 0) {
                for (long subNode : adjacencyList[currentNode]) {
                    if (nodesChainLength[subNode] == -1)
                        nodesToParseStack.push(subNode);
                }
                nodeChainsResult.back()++;
            }

            if (currentNode == terminatingNodes[currentTerminatingNode] ||
                currentNode == terminatingNodes[currentTerminatingNode - 1]) {
                nodeChainsResult.push_back(1);
                currentTerminatingNode--;
            }
        }
    }

    // Sorts the longest chains possible vector to be in ascending order
    sort(nodeChainsResult.begin(), nodeChainsResult.end());

    // Returns the longest chains possible result back to the calling code
    return nodeChainsResult;
}
