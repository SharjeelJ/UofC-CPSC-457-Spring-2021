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

    // Sets the partition iterator to be at the start of the partitions allocated list by default
    PartitionAddress memoryPartitionIterator = allocatedPartitions.begin();

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

        // Default
        memoryPartitionIterator = allocatedPartitions.begin();

        // If there is an empty partition present and that partition can fit requested partition
        if (!unallocatedPartitions.empty() && (*unallocatedPartitions.begin())->size >= size) {
            printf("Hit 1\n");
            // Defaults
            int64_t requestedPages = 0;
            int64_t requestedMemory = 0;
            int64_t existingMemory = 0;
            int64_t previousAddress = 0;

            // Stores the existing memory
            existingMemory = (*unallocatedPartitions.begin())->size;

            // Store requested pages needed
            requestedPages = ceil(double(size - existingMemory) / pageSize);

            // Store requested memory needed
            requestedMemory = requestedPages * pageSize;

            // Increment result
            result.n_pages_requested += requestedPages;

            previousAddress = (*unallocatedPartitions.begin())->address;

            // Sets pointer to be before the empty partition
            memoryPartitionIterator = prev(*unallocatedPartitions.begin());

            // Remove the existing empty partition
            allocatedPartitions.erase(*unallocatedPartitions.begin());
            unallocatedPartitions.erase(unallocatedPartitions.begin());

            // Add requested partition
            allocatedPartitions.insert(next(memoryPartitionIterator), Partition{tag, size, previousAddress});

            // Moves pointer to requested partition
            memoryPartitionIterator++;

            // Adds requested partition to the lookup table
            partitionLookupTable[tag].push_back(memoryPartitionIterator);

            // Add unused space partition (if any)
            if (existingMemory + requestedMemory - size != 0) {
                // Inserts empty partition
                allocatedPartitions.insert(next(memoryPartitionIterator),
                                           Partition{0, existingMemory + requestedMemory - size,
                                                     memoryPartitionIterator->address + memoryPartitionIterator->size});
                // Adds empty partition to the set
                unallocatedPartitions.insert(unallocatedPartitions.begin(), next(memoryPartitionIterator));
            }
        }
            // If there is an empty partition present at the end that can contribute to the new partition
        else if (!unallocatedPartitions.empty() && prev(allocatedPartitions.end())->tag == 0) {
            printf("Hit 2\n");
            // Defaults
            int64_t requestedPages = 0;
            int64_t requestedMemory = 0;
            int64_t existingMemory = 0;

            // Stores the existing memory
            existingMemory = prev(allocatedPartitions.end())->size;

            // Store requested pages needed
            requestedPages = ceil(double(size - existingMemory) / pageSize);

            // Store requested memory needed
            requestedMemory = requestedPages * pageSize;

            // Increment result
            result.n_pages_requested += requestedPages;

            // Remove the end partition
            unallocatedPartitions.erase(prev(allocatedPartitions.end()));
            allocatedPartitions.pop_back();

            // Set pointer to end partition
            memoryPartitionIterator = prev(allocatedPartitions.end());

            // Add requested partition
            if (allocatedPartitions.empty())
                allocatedPartitions.insert(next(memoryPartitionIterator), Partition{tag, size, 0});
            else
                allocatedPartitions.insert(next(memoryPartitionIterator), Partition{tag, size,
                                                                                    memoryPartitionIterator->address +
                                                                                    memoryPartitionIterator->size});

            // Moves pointer to requested partition
            memoryPartitionIterator++;

            // Adds requested partition to the lookup table
            partitionLookupTable[tag].push_back(memoryPartitionIterator);

            // Add unused space partition (if any)
            if (existingMemory + requestedMemory - size != 0) {
                // Inserts empty partition
                allocatedPartitions.insert(next(memoryPartitionIterator),
                                           Partition{0, existingMemory + requestedMemory - size,
                                                     memoryPartitionIterator->address + memoryPartitionIterator->size});
                // Adds empty partition to the set
                unallocatedPartitions.insert(unallocatedPartitions.begin(), --allocatedPartitions.end());
            }
        }
            // If there are no empty partitions at all
        else {
            printf("Hit 3\n");
            // Defaults
            int64_t requestedPages = 0;
            int64_t requestedMemory = 0;

            // Store requested pages needed
            requestedPages = ceil(double(size) / pageSize);

            // Store requested memory needed
            requestedMemory = requestedPages * pageSize;

            // Increment result
            result.n_pages_requested += requestedPages;

            // Set pointer to end partition
            memoryPartitionIterator = prev(allocatedPartitions.end());

            // Add requested partition
            if (allocatedPartitions.empty())
                allocatedPartitions.insert(next(memoryPartitionIterator), Partition{tag, size, 0});
            else
                allocatedPartitions.insert(next(memoryPartitionIterator), Partition{tag, size,
                                                                                    memoryPartitionIterator->address +
                                                                                    memoryPartitionIterator->size});

            // Moves pointer to requested partition
            memoryPartitionIterator++;

            // Adds requested partition to the lookup table
            partitionLookupTable[tag].push_back(memoryPartitionIterator);

            // Add unused space partition (if any)
            if (requestedMemory - size != 0) {
                // Inserts empty partition
                allocatedPartitions.insert(next(memoryPartitionIterator), Partition{0, requestedMemory - size,
                                                                                    memoryPartitionIterator->address +
                                                                                    memoryPartitionIterator->size});
                // Adds empty partition to the set
                unallocatedPartitions.insert(unallocatedPartitions.begin(), --allocatedPartitions.end());
            }
        }
    }

    // Function to deallocate memory
    void deallocate(int tag) {
        // Pseudocode for deallocation request:
        // - for every partition
        //     - if partition is occupied and has a matching tag:
        //         - mark the partition free
        //         - merge any adjacent free partitions

        // Checks to see if the tag has entries (otherwise skips the request)
        if (partitionLookupTable[tag].size() > 0) {
            // Loops through all partitions tied to the tag
            for (auto currentPartition : partitionLookupTable[tag]) {

                // Booleans to store if the front or end of the list was hit while finding adjacent empty partitions
                bool frontHit = false;
                bool endHit = false;

                // Sets the pointer to be at current partition
                memoryPartitionIterator = currentPartition;

                // Changes the current partition to now be an empty one
                currentPartition->tag = 0;

                // Stores all empty partitions to the left of the current partition
                PartitionAddress leftMostAdjacentUnallocatedPartition = currentPartition;
                while (!frontHit && leftMostAdjacentUnallocatedPartition->tag == 0) {
                    if (leftMostAdjacentUnallocatedPartition != allocatedPartitions.begin() &&
                        prev(leftMostAdjacentUnallocatedPartition)->tag == 0)
                        leftMostAdjacentUnallocatedPartition = prev(leftMostAdjacentUnallocatedPartition);
                    else if (leftMostAdjacentUnallocatedPartition == allocatedPartitions.begin())
                        frontHit = true;
                    else
                        break;
                }

                // Stores all empty partitions to the right of the current partition
                PartitionAddress rightMostAdjacentUnallocatedPartition = currentPartition;
                while (!endHit && rightMostAdjacentUnallocatedPartition->tag == 0) {
                    if (rightMostAdjacentUnallocatedPartition != prev(allocatedPartitions.end()) &&
                        next(rightMostAdjacentUnallocatedPartition)->tag == 0)
                        rightMostAdjacentUnallocatedPartition = next(rightMostAdjacentUnallocatedPartition);
                    else if (rightMostAdjacentUnallocatedPartition == prev(allocatedPartitions.end()))
                        endHit = true;
                    else
                        break;
                }
                unallocatedPartitions.insert(unallocatedPartitions.begin(), currentPartition);
            }
            partitionLookupTable.erase(tag);
        }
    }

    // Function to return the results back to the calling code based on the current memory partitioning
    MemSimResult getStats() {
        result.max_free_partition_address = (*unallocatedPartitions.begin())->address;
        result.max_free_partition_size = (*unallocatedPartitions.begin())->size;
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
