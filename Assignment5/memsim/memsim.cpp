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


        printf("\nInput: %ld) %ld\n", tag, size);
        printf("Print: %ld\n", allocatedPartitions.size());
        for (auto x : allocatedPartitions)
            printf("%ld) %ld %ld\n", x.tag, x.address, x.size);

        // Default
        memoryPartitionIterator = allocatedPartitions.begin();

        // Stores if an existing unallocated partition can be used
        bool canUseExistingPartition = false;
        if (!unallocatedPartitions.empty() && (*unallocatedPartitions.begin())->size >= size) {
            canUseExistingPartition = true;
            memoryPartitionIterator = (*unallocatedPartitions.begin());
            memoryPartitionIterator--;
            unallocatedPartitions.erase(next(memoryPartitionIterator));
            allocatedPartitions.erase(next(memoryPartitionIterator));
        }

        // Stores the total number of page(s) that need to be requested (if any), the total memory that will be requested (if any) and increments the counter keeping track of page requests (if necessary)
        int64_t requestedPages = 0;
        int64_t requestedMemory = 0;
        int64_t endPartitionMemory = 0;
        bool usedEndPartition = false;
//        if (size > pageSize && !canUseExistingPartition)
        if (!canUseExistingPartition) {
            // Checks to see if there is an unallocated partition at the end that we can use
            printf("Test: %ld\n", prev(allocatedPartitions.end())->tag);
            if (prev(allocatedPartitions.end())->tag != -1 || unallocatedPartitions.empty())
                requestedPages = ceil(double(size) / pageSize);
            else if (prev(allocatedPartitions.end())->tag == -1) {
                usedEndPartition = true;
                endPartitionMemory = prev(allocatedPartitions.end())->size;
                requestedPages = ceil(double(abs(size - prev(allocatedPartitions.end())->size)) / pageSize);
                unallocatedPartitions.erase(prev(allocatedPartitions.end()));
                allocatedPartitions.pop_back();
            }
            requestedMemory = requestedPages * pageSize;
            result.n_pages_requested += requestedPages;
            memoryPartitionIterator = prev(allocatedPartitions.end());
        }

        // Creates a partition based on the specified tag and size
        if (!allocatedPartitions.empty())
            allocatedPartitions.insert(next(memoryPartitionIterator),
                                       Partition{tag, size,
                                                 memoryPartitionIterator->address + memoryPartitionIterator->size});
        else
            allocatedPartitions.insert(memoryPartitionIterator, Partition{tag, size, 0});


        // Updates the iterator to point to the newly created partition
        memoryPartitionIterator++;

        // Adds the created partition's address to the partition lookup table using its tag as the key
        partitionLookupTable[tag].push_back(memoryPartitionIterator);

        // Inserts the unallocated space generated (if any) as a partition to the right side of the created partition
        if (!canUseExistingPartition && size > pageSize && requestedPages > 0) {
            // Creates a partition for the unallocated space
            allocatedPartitions.insert(next(memoryPartitionIterator), Partition{-1, requestedMemory - size,
                                                                                memoryPartitionIterator->address +
                                                                                memoryPartitionIterator->size});

            // Stores the unallocated partition in a sorted set (will assist in finding unallocated space partitions in the future)
            unallocatedPartitions.insert(unallocatedPartitions.begin(), --allocatedPartitions.end());
        } else if (!canUseExistingPartition && size < pageSize && requestedPages == 0) {
            // Creates a partition for the unallocated space
            allocatedPartitions.insert(next(memoryPartitionIterator), Partition{-1, pageSize - size,
                                                                                memoryPartitionIterator->address +
                                                                                memoryPartitionIterator->size});

            // Stores the unallocated partition in a sorted set (will assist in finding unallocated space partitions in the future)
            unallocatedPartitions.insert(unallocatedPartitions.begin(), --allocatedPartitions.end());
        } else if (canUseExistingPartition && !unallocatedPartitions.empty() &&
                   size < (*unallocatedPartitions.begin())->size) {
            // Creates a partition for the unallocated space
            allocatedPartitions.insert(next(memoryPartitionIterator), Partition{-1, pageSize - size,
                                                                                memoryPartitionIterator->address +
                                                                                memoryPartitionIterator->size});

            // Stores the unallocated partition in a sorted set (will assist in finding unallocated space partitions in the future)
            unallocatedPartitions.insert(unallocatedPartitions.begin(), --allocatedPartitions.end());
        } else if (usedEndPartition) {
            // Creates a partition for the unallocated space
            allocatedPartitions.insert(next(memoryPartitionIterator),
                                       Partition{-1, requestedMemory + endPartitionMemory - size,
                                                 memoryPartitionIterator->address +
                                                 memoryPartitionIterator->size});

            // Stores the unallocated partition in a sorted set (will assist in finding unallocated space partitions in the future)
            unallocatedPartitions.insert(unallocatedPartitions.begin(), --allocatedPartitions.end());
        }

        // Code run if an existing partition already exists
//        else {
//            // Gets the largest unallocated partition based on size (gets the partition with the smallest tag in the case of a tie)
//            auto largestUnallocatedPartition = unallocatedPartitions.begin();
//
//            // Stores the total number of pages that need to be requested
//            int64_t requestedPages = ceil(double(abs((*largestUnallocatedPartition)->size - size)) / pageSize);
//
//            // Stores the total size of the partition being added
//            int64_t requestedMemory = requestedPages * pageSize;
//
//            // Checks to see if the request can be fit into an existing partition
//            if ((*largestUnallocatedPartition)->size >= size) {
//                // Stores the existing largest unallocated partition's free space
//                int64_t existingUnallocatedSize = (*largestUnallocatedPartition)->size;
//
//                // Sets the memory partition iterator to be the partition where insertion will take place
//                memoryPartitionIterator = *largestUnallocatedPartition;
//
//                // Decrements the iterator to now point to the previous partition (non empty)
//                memoryPartitionIterator.operator--();
//
//                // Removes the free space partition
//                unallocatedPartitions.erase(largestUnallocatedPartition);
//                allocatedPartitions.erase(*largestUnallocatedPartition);
//
//                // Creates a partition based on the specified tag and size at the previous empty partition address
//                allocatedPartitions.insert(next(memoryPartitionIterator), Partition{tag, size,
//                                                                                    memoryPartitionIterator->address +
//                                                                                    memoryPartitionIterator->size});
//
//                // Updates the iterator to point to the newly created partition
//                memoryPartitionIterator.operator++();
//
//                // Adds the partition's address to the partition lookup table using its tag as the key
//                partitionLookupTable[tag].push_back(memoryPartitionIterator);
//
//                // Checks to see if an empty partition needs to be inserted
//                if (existingUnallocatedSize > size) {
//                    // Inserts the unused space as an empty partition right after the new partition
//                    allocatedPartitions.insert(next(memoryPartitionIterator),
//                                               Partition{-1, existingUnallocatedSize - size,
//                                                         memoryPartitionIterator->address +
//                                                         memoryPartitionIterator->size});
//
//                    // Stores the unused space
//                    unallocatedPartitions.insert(unallocatedPartitions.begin(), --allocatedPartitions.end());
//
//                    // Increments the iterator to now point to the empty space partition
//                    memoryPartitionIterator.operator++();
//                }
//            }
//                // Code run if more pages needed to be requested (means that the partition we will insert is at the end)
//            else {
//                // Increments how many pages had to be requested in total
//                result.n_pages_requested += requestedPages;
//
//                // Stores the existing largest unallocated partition's free space
//                int64_t existingUnallocatedSize = (*largestUnallocatedPartition)->size;
//
//                // Decrements the iterator to now point to the previous partition (non empty)
//                memoryPartitionIterator.operator--();
//
//                // Removes the free space partition
//                unallocatedPartitions.erase(largestUnallocatedPartition);
//                allocatedPartitions.erase(*largestUnallocatedPartition);
//
//                // Creates a partition based on the specified tag and size at the end
//                allocatedPartitions.push_back(
//                        Partition{tag, size, memoryPartitionIterator->address + memoryPartitionIterator->size});
//
//                // Updates the iterator to point to the newly created partition
//                memoryPartitionIterator.operator++();
//
//                // Adds the partition's address to the partition lookup table using its tag as the key
//                partitionLookupTable[tag].push_back(memoryPartitionIterator);
//
//                // Inserts the unused space generated when a new page was created as an empty partition
//                allocatedPartitions.push_back(
//                        Partition{-1, existingUnallocatedSize + requestedMemory - size,
//                                  memoryPartitionIterator->address +
//                                  memoryPartitionIterator->size});
//
//                // Stores the unused space generated when a new page was created
//                unallocatedPartitions.insert(unallocatedPartitions.begin(), --allocatedPartitions.end());
//
//                // Increments the iterator to now point to the empty space partition
//                memoryPartitionIterator.operator++();
//            }
//        }
    }

    // Function to deallocate memory
    void deallocate(int tag) {
        // Pseudocode for deallocation request:
        // - for every partition
        //     - if partition is occupied and has a matching tag:
        //         - mark the partition free
        //         - merge any adjacent free partitions

//        for (PartitionAddress currentPartition : partitionLookupTable[tag]) {
//            memoryPartitionIterator = currentPartition;
//            allocatedPartitions.erase(currentPartition);
//
//            unallocatedPartitions.insert(unallocatedPartitions.begin(), --allocatedPartitions.end());
//        }
//        partitionLookupTable.erase(tag);
    }

    // Function to return the results back to the calling code based on the current memory partitioning
    MemSimResult getStats() {
        result.max_free_partition_address = memoryPartitionIterator->address;
        result.max_free_partition_size = memoryPartitionIterator->size;
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
