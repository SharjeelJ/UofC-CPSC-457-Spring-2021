#include "memsim.h"
#include <iostream>
#include <list>
#include <set>
#include <unordered_map>
#include <cmath>

using namespace std;

// Creates a struct that will store the tag, size and address of each partition
struct Partition {
    int tag;
    int64_t size, address;
};

// Creates an iterator for the doubly linked list consisting of Partition structs
typedef list<Partition>::iterator PartitionAddress;

// Custom comparator struct that will be used to sort free partitions
struct customComparator {
    bool operator()(const PartitionAddress &c1, const PartitionAddress &c2) const {
        if (c1->size == c2->size)
            return c1->address < c2->address;
        else
            return c1->size > c2->size;
    }
};

// Struct that will simulate memory handling
struct Simulator {
    // Doubly linked list that will store all the memory partitions (by default has a single partition of size 0)
    list<Partition> allocatedPartitions;

    // Set containing all the memory partitions from the doubly linked list in sorted order based on the size/address
    set<PartitionAddress, customComparator> unallocatedPartitions;

    // Unordered map that will store all the partitions tied to a specific tag
    unordered_map<long, vector<PartitionAddress>> partitionLookupTable;

    // Stores the page size value specified by the calling code (will determine partition sizes)
    int64_t pageSize = 0;

    // Sets the iterator keeping track of the last memory partition's address
    PartitionAddress lastMemoryPartitionAddress = allocatedPartitions.begin();

    // Creates the result struct that will store the results if the calling code requests them and stores default values into its elements
    MemSimResult result = {0, 0, 0};

    // Constructor that will store the page file sized specified by the calling code
    Simulator(int64_t page_size) {
        pageSize = page_size;
    }

    // Function to allocate memory
    void allocate(int tag, int size) {
        // Pseudocode for allocation request:
        // - search through the list of partitions from start to end, and
        //   find the largest partition that fits requested size
        //     - in case of ties, pick the first partition found
        // - if no suitable partition found:
        //     - get minimum number of pages from OS, but consider the
        //       case when last partition is free
        //     - add the new memory at the end of partition list
        //     - the last partition will be the best partition
        // - split the best partition in two if necessary
        //     - mark the first partition occupied, and store the tag in it
        //     - mark the second partition free

        // Checks to see if the current memory allocation call is the first call since the simulator was initialized
        if (allocatedPartitions.empty()) {
            // Stores the total number of pages that need to be requested
            int64_t requestedPages = ceil(double(size) / pageSize);

            // Stores the total size of the partition being added
            int64_t requestedMemory = requestedPages * pageSize;

            // Increments how many pages had to be requested in total
            result.n_pages_requested += requestedPages;

            // Creates a partition based on the specified tag and size
            allocatedPartitions.push_back(Partition{tag, size, lastMemoryPartitionAddress->address});

            // Updates the address to point to the newly created first partition
            lastMemoryPartitionAddress = allocatedPartitions.begin();

            // Adds the partition's address to the partition lookup table using its tag as the key
            partitionLookupTable[tag].push_back(lastMemoryPartitionAddress);

            // Inserts the unused space generated when a new page was created as an empty partition
            allocatedPartitions.push_back(Partition{-1, requestedMemory - size, lastMemoryPartitionAddress->address +
                                                                                lastMemoryPartitionAddress->size});

            // Stores the unused space generated when a new page was created
            unallocatedPartitions.insert(unallocatedPartitions.begin(), --allocatedPartitions.end());

            // Increments the iterator to now point to the empty space partition
            lastMemoryPartitionAddress.operator++();
        }
            // Code run if an existing partition already exists
        else {

        }
    }

    // Function to deallocate memory
    void deallocate(int tag) {
        // Pseudocode for deallocation request:
        // - for every partition
        //     - if partition is occupied and has a matching tag:
        //         - mark the partition free
        //         - merge any adjacent free partitions
    }

    // Function to return the results back to the calling code based on the current memory partitioning
    MemSimResult getStats() {
        result.max_free_partition_address = lastMemoryPartitionAddress->address;
        result.max_free_partition_size = lastMemoryPartitionAddress->size;
        return result;
    }

    // Function to check the consistency of the current memory partitioning
    void check_consistency() {
        // make sure the sum of all partition sizes in your linked list is
        // the same as number of page requests * pageSize

        // make sure your addresses are correct

        // make sure the number of all partitions in your tag data structure +
        // number of partitions in your free blocks is the same as the size
        // of the linked list

        // make sure that every free partition is in free blocks

        // make sure that every partition in unallocatedPartitions is actually free

        // make sure that none of the partition sizes or addresses are < 1
    }
};


// re-implement the following function
// ===================================
// parameters:
//    page_size: integer in range [1..1,000,000]
//    requests: array of requests
// return:
//    some statistics at the end of simulation
MemSimResult mem_sim(int64_t page_size, const std::vector<Request> &requests) {
    Simulator sim(page_size);
    for (const auto &req : requests) {
        if (req.tag < 0) {
            sim.deallocate(-req.tag);
        } else {
            sim.allocate(req.tag, req.size);
        }
        sim.check_consistency();
    }
    return sim.getStats();
}
