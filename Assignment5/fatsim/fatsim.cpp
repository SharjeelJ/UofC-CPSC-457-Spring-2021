#include "fatsim.h"
#include <algorithm>
#include <stack>

using namespace std;

/**
 * Function that uses the provided fat vector and returns the longest chains formed by the node's that lead to a terminating node
 * @note Implements code from fatsim-w21 (https://gitlab.com/cpsc457/public/fatsim-w21)
 * @param fat - Pointer to a vector consisting of the fat node data
 * @return vector<long> - Vector containing the largest chain possible from each of the node's that lead to a terminating node in ascending order
 */
std::vector<long> fat_check(const std::vector<long> &fat) {
    // Creates an adjacency list of the size of the passed in vector
    vector<vector<long>> adjacencyList(fat.size());

    // Creates a vector to store the chain length of each node (so stores the maximum chain length of all its sub-nodes) with a default value for -1 for each node (will help in indicating which nodes have been parsed as -1 = unparsed, 0 = parsed but not a leaf node (will be revisited to get the sub-node data), 1 = leaf node)
    vector<long> nodesChainLength(fat.size(), -1);

    // Creates a vector consisting of all nodes leading to a terminating node
    vector<long> terminatingNodes;

    // Populates the adjacency list as well as the vector containing which nodes lead to a terminating node
    for (int currentNode = 0; currentNode < fat.size(); currentNode++)
        // Checks to see if the current currentNode leads to a terminating currentNode and adds it to the appropriate vector/list
        if (fat[currentNode] == -1)
            terminatingNodes.push_back(currentNode);
        else
            adjacencyList[fat[currentNode]].push_back(currentNode);

    // Creates the vector that will store the length of all possible chains leading up to a terminating node (will be returned to the calling code once populated)
    vector<long> nodeChainsResult;

    // Creates a stack to store the nodes that need to still be parsed
    stack<long> nodesToParseStack;

    // Populates the stack with the known nodes that lead to terminating nodes
    for (long currentNode : terminatingNodes)
        nodesToParseStack.push(currentNode);

    // Checks to see if there is at least 1 node that leads to a terminating node otherwise skips running DFS
    if (!terminatingNodes.empty()) {
        // Performs DFS and populates the result vector
        while (!nodesToParseStack.empty()) {
            // Stores the current node that will be parsed and removes it from the stack
            long currentNode = nodesToParseStack.top();
            nodesToParseStack.pop();

            // Checks to see if the current node is being visited for the first time or additional time (to this time use its sub-nodes data to determine its own)
            if (nodesChainLength[currentNode] == -1) {
                // Updates the current node's length (marks it as visited)
                nodesChainLength[currentNode] = 0;

                // Checks to see if there are any immediate sub-nodes of the current node
                if (adjacencyList[currentNode].size() > 0) {
                    // Pushes back the original parent node (will be revisited once all its sub-nodes are populated)
                    nodesToParseStack.push(currentNode);

                    // Populates the stack with all the immediate sub-nodes of the current node
                    for (long subNode : adjacencyList[currentNode])
                        // Only adds sub-nodes to the stack that have not previously been visited
                        if (nodesChainLength[subNode] == -1)
                            nodesToParseStack.push(subNode);
                } else
                    // Increments the node length to indicate that the current node is a leaf node
                    nodesChainLength[currentNode] = 1;
            } else {
                // Stores the longest length reported by the current node's sub-node(s)
                long longestSubNodeChain = 0;

                // Parses through the data of each of the sub-nodes
                for (long subNode : adjacencyList[currentNode])
                    // Only stores the current node's data if it is larger than the existing data
                    if (nodesChainLength[subNode] > longestSubNodeChain)
                        longestSubNodeChain = nodesChainLength[subNode];

                // Increments the current node (parent) with the data collected from its sub-node(s)
                nodesChainLength[currentNode] += longestSubNodeChain + 1;
            }
        }
    }

    // Loops through all the known terminating nodes and populates the result vector with their collected data
    for (long currentNode : terminatingNodes)
        nodeChainsResult.push_back(nodesChainLength[currentNode]);

    // Sorts the longest chains possible vector to be in ascending order
    sort(nodeChainsResult.begin(), nodeChainsResult.end());

    // Returns the longest chains possible result back to the calling code
    return nodeChainsResult;
}
