#include "deadlock_detector.h"
#include "common.h"

using namespace std;

// Class that will be used to generate a graph that will be topologically sorted to find cycles in a single instance per resource type system
class FastGraph {
public:
    vector<vector<int>> adjacencyList;
    vector<int> outDegree;
};

/**
 * Function that uses the provided edges vector to create a graph and checks for deadlocks via topological sort after each edge is inserted
 * @note Implements code from deadlock-detect (https://gitlab.com/cpsc457/public/deadlock-detect)
 * @param edges - Pointer to a string vector composed of a string for the process and resource alongside a string indicating whether a request or assignment is occurring
 * @return result - Result struct where dl_procs are all processes current in deadlock and edge_index is the edge responsible for the deadlock
 */
Result detect_deadlock(const std::vector<std::string> &edges) {
    // Initialize a object that will store the results (what edge caused a deadlock and which processes are in deadlock)
    Result result;

    // Sets the default value of the result to indicate no cycles were detected
    result.edge_index = -1;

    // Creates a new graph object that will store the adjacency list based on the passed in input alongside each node's out degree
    FastGraph graph;

    // Initialize a converter object whose job would be to take in the passed in process and resource strings and convert them to unique integers instead
    Word2Int stringConverter;

    // Initialize an unordered map that will store the unique integer ids for all the processes and resources while also storing their original string
    unordered_map<int, string> conversionRecord;

    // Loops through all the edges provided and populates the graph (adjacency list and out degree vector)
    for (int counter = 0; counter < int(edges.size()); counter++) {
        // Vector to store parts of the current string being parsed (in the form of process, operator, resource)
        vector<string> cleanedStringParts = split(simplify(edges[counter]));

        // Stores the current string's process, operator and resource node data
        int process = stringConverter.get("P" + cleanedStringParts[0]);
        conversionRecord[process] = "P" + cleanedStringParts[0];
        string activity = cleanedStringParts[1];
        int resource = stringConverter.get("R" + cleanedStringParts[2]);
        conversionRecord[resource] = "R" + cleanedStringParts[2];

        // Checks to see if 2 or 1 new blank entries need to be added to the graph adjacency list and out degree vector (based on how many unique nodes we will be handling this iteration)
        if (int(graph.adjacencyList.size()) - process == -1 || int(graph.adjacencyList.size()) - resource == -1) {
            graph.adjacencyList.emplace_back();
            graph.adjacencyList.emplace_back();
            graph.outDegree.push_back(0);
            graph.outDegree.push_back(0);
        } else if (int(graph.adjacencyList.size()) - process == 0 || int(graph.adjacencyList.size()) - resource == 0) {
            graph.adjacencyList.emplace_back();
            graph.outDegree.push_back(0);
        }

        // Populates the adjacency list and out degree vector appropriately based on whether the current string is an request or assignment operation
        if (activity == "->") {
            graph.adjacencyList[resource].push_back(process);
            graph.outDegree[process]++;
        } else if (activity == "<-") {
            graph.adjacencyList[process].push_back(resource);
            graph.outDegree[resource]++;
        }

        // Stores a local copy of the graph's out degree vector so that it can be modified safely for cycle detection
        vector<int> outDegreeTemp = graph.outDegree;

        // Creates and populates a vector that will store all nodes with an out degree value of 0
        vector<int> outDegreeZeroNodes;
        for (int innerCounter = 0; innerCounter < int(outDegreeTemp.size()); innerCounter++)
            if (outDegreeTemp[innerCounter] == 0)
                outDegreeZeroNodes.push_back(innerCounter);

        // Loops through all nodes that had an out degree value of zero
        while (!outDegreeZeroNodes.empty()) {
            // Stores and pops the last element from the vector of nodes with out degree zero
            int zeroNode = outDegreeZeroNodes.back();
            outDegreeZeroNodes.pop_back();

            // Decrements the out degree value of any nodes pointing to the current node (that was popped) and pops them out as well if they are now at out degree zero
            for (int currentNode : graph.adjacencyList[zeroNode]) {
                outDegreeTemp[currentNode]--;
                if (outDegreeTemp[currentNode] == 0)
                    outDegreeZeroNodes.push_back(currentNode);
            }
        }

        // Loops through the shrunk graph and populates the result object if any deadlocked processes are found
        for (int innerCounter = 0; innerCounter <= int(outDegreeTemp.size()); innerCounter++) {
            if (outDegreeTemp[innerCounter] > 0 && conversionRecord[innerCounter].substr(0, 1) == "P") {
                result.edge_index = counter;
                result.dl_procs.push_back(conversionRecord[innerCounter].substr(1));
            }
        }

        // If any deadlocked processes were found then stops populating the graph further
        if (!result.dl_procs.empty())
            break;
    }

    // Returns the result back to the calling code
    return result;
}
