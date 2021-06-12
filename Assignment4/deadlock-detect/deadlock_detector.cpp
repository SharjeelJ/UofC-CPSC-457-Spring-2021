#include "deadlock_detector.h"
#include "common.h"

using namespace std;

class FastGraph {
public:
    vector<vector<int>> adjacencyList;
    vector<int> outDegree;
};

/// this is the function you need to (re)implement
///
/// parameter edges[] contains a list of request- and assignment- edges
///   example of a request edge, process "p1" resource "r1"
///     "p1 -> r1"
///   example of an assignment edge, process "XYz" resource "XYz"
///     "XYz <- XYz"
///
/// You need to process edges[] one edge at a time, and run a deadlock
/// detection after each edge. As soon as you detect a deadlock, your function
/// needs to stop processing edges and return an instance of Result structure
/// with edge_index set to the index that caused the deadlock, and dl_procs set
/// to contain with names of processes that are in the deadlock.
///
/// To indicate no deadlock was detected after processing all edges, you must
/// return Result with edge_index = -1 and empty dl_procs[].
///
Result detect_deadlock(const std::vector<std::string> &edges) {
    // Initialize a vector that will store the results (what edge caused a deadlock and which processes are in deadlock)
    Result result;

    // Creates a new graph object that will store the adjacency list based on the passed in input
    FastGraph graph;

    // Initialize a stringConverter object whose job would be to take in the passed in strings and convert them to unique integers instead
    Word2Int stringConverter;

    // Loops through all the edges provided and populates the graph (adjacency list and out degree vector)
    for (int counter = 0; counter < edges.size(); counter++) {
        // Vector to store parts of the current string being parsed (in the form of process, operator, resource)
        vector<string> cleanedStringParts = split(simplify(edges[counter]));

        // Stores the current string's process, operator and resource node data
        int process = stringConverter.get("p" + cleanedStringParts[0]);
        string activity = cleanedStringParts[1];
        int resource = stringConverter.get("r" + cleanedStringParts[2]);

        // Checks to see if 2 or 1 new blank entries need to be added to the graph adjacency list and out degree vector (based on how many unique nodes we will be handling this iteration)
        if (graph.adjacencyList.size() - process == -1 || graph.adjacencyList.size() - resource == -1) {
            graph.adjacencyList.push_back(vector<int>());
            graph.adjacencyList.push_back(vector<int>());
            graph.outDegree.push_back(0);
            graph.outDegree.push_back(0);
        } else if (graph.adjacencyList.size() - process == 0 || graph.adjacencyList.size() - resource == 0) {
            graph.adjacencyList.push_back(vector<int>());
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

        // TODO: Remove test print
//        printf("%s\n", edges[counter].c_str());
        printf("%d %s %d\n\n", process, activity.c_str(), resource);
    }

    // Stores a local copy of the graph's out degree vector so that it can be modified
//    vector<int> out = graph.outDegree;

    // Creates and populates a vector that will store all nodes with an out degree value of 0
    vector<int> zeroes;
    for (int counter = 0; counter < graph.outDegree.size(); counter++)
        if (graph.outDegree[counter] == 0)
            zeroes.push_back(counter);

    // Loops through all nodes that had an out degree value of zero
    while (!zeroes.empty()) {
        // Pops and stores the last element from the zeroes list
        int zeroNode = zeroes.back();
        zeroes.pop_back();

        for (int currentNode : graph.adjacencyList[zeroNode]) {
            graph.outDegree[currentNode]--;
            if (graph.outDegree[currentNode] == 0)
                zeroes.push_back(currentNode);
        }
    }

    for (int counter = 0; counter < edges.size(); counter++) {
        // Vector to store parts of the current string being parsed (in the form of process, operator, resource)
        vector<string> cleanedStringParts = split(simplify(edges[counter]));

        // Stores the current string's process, operator and resource node data
        int process = stringConverter.get("p" + cleanedStringParts[0]);

        if (graph.outDegree[process] > 0)
            result.dl_procs.push_back(cleanedStringParts[0]);
    }

    /*
    out = outDegree;
    zeroes[] = find all nodes in graph with outdegree == 0
     while zeroes is not empty:
        n = remove one entry from zeroes[]
    for every n2 of adjacencyList[n]:
         out[n2] --
         if out[n2] == 0:
            append n2 to zeroes[]
    dl_procs[] = all nodes n that represent a process and out[n]>0
    */

    if (result.dl_procs.size() == 0)
        result.edge_index = -1;

    return result;
}
