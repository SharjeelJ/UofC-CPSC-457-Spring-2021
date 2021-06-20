#include "memsim.h"
#include <iostream>
#include <list>
#include <set>
#include <unordered_map>
#include <cmath>

using namespace std;

// Struct that will store the tag, size and address of each partition
struct Partition {
    int tag;
    int64_t size;
    int64_t address;
};

// Creates an iterator for the doubly linked list consisting of Partition structs
typedef list<Partition>::iterator PartitionAddress;

// Custom comparator struct that will be used to sort unallocated partitions
struct customComparator {
    /**
     * Function that acts as a comparator and compares two Partition's based on their size and falls back to their address in the case of a tie
     * @note Implements code from memsim-w21 (https://gitlab.com/cpsc457/public/memsim-w21)
     * @param c1 - Pointer to the first Partition
     * @param c2 - Pointer to the second Partition
     * @returns Boolean - True if (Partition 1 size > Partition 2 size) OR (Partition 1 address < Partition 2 address AND Partition 1 size == Partition 2 size)
     */
    bool operator()(const PartitionAddress &c1, const PartitionAddress &c2) const {
        if (c1->size == c2->size)
            return c1->address < c2->address;
        else
            return c1->size > c2->size;
    }
};

// Struct that will simulate memory handling
struct Simulator {
    // Doubly linked list that will store all the memory partitions
    list<Partition> allPartitions;

    // Set containing all the unallocated memory partitions from the doubly linked list in sorted order based on the size/address (partition that has the smallest address is preferred in the case of a tie)
    set<PartitionAddress, customComparator> unallocatedPartitions;

    // Unordered map that will store all the partitions tied to a specific tag
    unordered_map<long, vector<PartitionAddress>> partitionLookupTable;

    // Stores the page size value specified by the calling code (will determine partition sizes)
    int64_t pageSize = 0;

    // Sets the partition iterator to be at the start of the partitions list by default
    PartitionAddress partitionIterator = allPartitions.begin();

    // Creates the result struct that will store the results if the calling code requests them and stores default values into it
    MemSimResult result = {0, 0, 0};

    // Constructor that will store the page size specified by the calling code
    /**
     * Constructor that sets the page size that will be used for all future partition allocation / deallocation actions
     * @note Implements code from memsim-w21 (https://gitlab.com/cpsc457/public/memsim-w21)
     * @param page_size - Page size to use for all future partition actions
     */
    explicit Simulator(int64_t page_size) {
        pageSize = page_size;
    }

    /**
     * Function that inserts the specified tag and size as a partition
     * @note Implements code from memsim-w21 (https://gitlab.com/cpsc457/public/memsim-w21)
     * @param tag - Integer tag of the requested partition to be created
     * @param size - Integer size of the requested partition to be created
     */
    void allocate(int tag, int size) {
        // Sets the partition iterator to the first partition
        partitionIterator = allPartitions.begin();

        // Checks to see if there is an empty partition present and that partition can fit requested partition
        if (!unallocatedPartitions.empty() && (*unallocatedPartitions.begin())->size >= size) {
            // Default values that will be used in the allocation
            int64_t requestedPages;
            int64_t requestedMemory;
            int64_t existingMemory;
            int64_t existingPartitionAddress;

            // Stores the size of the partition that will be inserted into
            existingMemory = (*unallocatedPartitions.begin())->size;

            // Stores the number of pages that need to be requested
            requestedPages = ceil(double(size - existingMemory) / pageSize);

            // Changes the number of pages that need to be requested to 0 if it was a negative value (can occur when the existing partition's size is greater than the new partition will need)
            if (requestedPages < 0)
                requestedPages = 0;

            // Stores the total amount of memory that will be requested
            requestedMemory = requestedPages * pageSize;

            // Increments the total number of pages that were allocated during this runtime
            result.n_pages_requested += requestedPages;

            // Stores the address value of the existing partition
            existingPartitionAddress = (*unallocatedPartitions.begin())->address;

            // Sets the iterator to be before the unallocated partition (prevents issues due to the memory being modified while being pointed to by the iterator)
            partitionIterator = prev(*unallocatedPartitions.begin());

            // Remove the existing unallocated partition
            allPartitions.erase(*unallocatedPartitions.begin());
            unallocatedPartitions.erase(unallocatedPartitions.begin());

            // Adds the new requested partition
            allPartitions.insert(next(partitionIterator), Partition{tag, size, existingPartitionAddress});

            // Increments the iterator to now point to the newly created partition
            partitionIterator++;

            // Adds the newly created partition to the partition lookup table based on its tag value being the key
            partitionLookupTable[tag].push_back(partitionIterator);

            // Checks to see if an unallocated space partition needs to be created
            if (existingMemory + requestedMemory - size != 0) {
                // Inserts an unallocated partition to the right of the newly generated partition
                allPartitions.insert(next(partitionIterator), Partition{-1, existingMemory + requestedMemory - size,
                                                                        partitionIterator->address +
                                                                        partitionIterator->size});

                // Adds the newly created unallocated partition to the unallocated partitions set
                unallocatedPartitions.insert(unallocatedPartitions.begin(), next(partitionIterator));
            }
        }
            // Checks to see if there is an unallocated partition present at the end that can contribute to the requested partition
        else if (!unallocatedPartitions.empty() && prev(allPartitions.end())->tag == -1) {
            // Default values that will be used in the allocation
            int64_t requestedPages;
            int64_t requestedMemory;
            int64_t existingMemory;

            // Stores the size of the unallocated partition that will be consumed
            existingMemory = prev(allPartitions.end())->size;

            // Stores the number of pages that need to be requested
            requestedPages = ceil(double(size - existingMemory) / pageSize);

            // Stores the total amount of memory that will be requested
            requestedMemory = requestedPages * pageSize;

            // Increments the total number of pages that were allocated during this runtime
            result.n_pages_requested += requestedPages;

            // Remove the existing unallocated partition (end partition)
            unallocatedPartitions.erase(prev(allPartitions.end()));
            allPartitions.pop_back();

            // Sets the iterator to point to end partition
            partitionIterator = prev(allPartitions.end());

            // Adds the requested partition based on if it's the only partition in the list or there are pre-existing partitions as well
            if (allPartitions.empty())
                allPartitions.insert(next(partitionIterator), Partition{tag, size, 0});
            else
                allPartitions.insert(next(partitionIterator),
                                     Partition{tag, size, partitionIterator->address + partitionIterator->size});

            // Increments the iterator to now point to the newly created partition
            partitionIterator++;

            // Adds the newly created partition to the partition lookup table based on its tag value being the key
            partitionLookupTable[tag].push_back(partitionIterator);

            // Checks to see if an unallocated space partition needs to be created
            if (existingMemory + requestedMemory - size != 0) {
                // Inserts an unallocated partition to the right of the newly generated partition (end)
                allPartitions.insert(next(partitionIterator), Partition{-1, existingMemory + requestedMemory - size,
                                                                        partitionIterator->address +
                                                                        partitionIterator->size});

                // Adds the newly created unallocated partition to the unallocated partitions set
                unallocatedPartitions.insert(unallocatedPartitions.begin(), --allPartitions.end());
            }
        }
            // Run if there are no empty partitions at all that can be used
        else {
            // Default values that will be used in the allocation
            int64_t requestedPages;
            int64_t requestedMemory;

            // Stores the number of pages that need to be requested
            requestedPages = ceil(double(size) / pageSize);

            // Stores the total amount of memory that will be requested
            requestedMemory = requestedPages * pageSize;

            // Increments the total number of pages that were allocated during this runtime
            result.n_pages_requested += requestedPages;

            // Sets the iterator to point to end partition
            partitionIterator = prev(allPartitions.end());

            // Adds the requested partition based on if it's the only partition in the list or there are pre-existing partitions as well
            if (allPartitions.empty())
                allPartitions.insert(next(partitionIterator), Partition{tag, size, 0});
            else
                allPartitions.insert(next(partitionIterator),
                                     Partition{tag, size, partitionIterator->address + partitionIterator->size});

            // Increments the iterator to now point to the newly created partition
            partitionIterator++;

            // Adds the newly created partition to the partition lookup table based on its tag value being the key
            partitionLookupTable[tag].push_back(partitionIterator);

            // Checks to see if an unallocated space partition needs to be created
            if (requestedMemory - size != 0) {
                // Inserts an unallocated partition to the right of the newly generated partition (end)
                allPartitions.insert(next(partitionIterator), Partition{-1, requestedMemory - size,
                                                                        partitionIterator->address +
                                                                        partitionIterator->size});

                // Adds the newly created unallocated partition to the unallocated partitions set
                unallocatedPartitions.insert(unallocatedPartitions.begin(), --allPartitions.end());
            }
        }
    }

    /**
     * Function that erases the specified tag's associated partition(s)
     * @note Implements code from memsim-w21 (https://gitlab.com/cpsc457/public/memsim-w21)
     * @param tag - Integer tag of partition(s) to be erased
     */
    void deallocate(int tag) {
        // Checks to see if the specified tag has any associated partitions (otherwise skips the request)
        if (partitionLookupTable[tag].size() > 0) {
            // Loops through all partitions tied to the tag
            for (auto currentPartition : partitionLookupTable[tag]) {
                // Changes the current partition to now be unallocated
                currentPartition->tag = -1;

                // Merges all unallocated partitions that are to the left of the current partition (up until a non empty partition is encountered or the edge of the partitions list)
                while (allPartitions.begin() != currentPartition && prev(currentPartition)->tag == -1) {
                    currentPartition->address = prev(currentPartition)->address;
                    currentPartition->size += prev(currentPartition)->size;
                    unallocatedPartitions.erase(prev(currentPartition));
                    allPartitions.erase(prev(currentPartition));
                }

                // Merges all unallocated partitions that are to the right of the current partition (up until a non empty partition is encountered or the edge of the partitions list)
                while (prev(allPartitions.end()) != currentPartition && next(currentPartition)->tag == -1) {
                    currentPartition->size += next(currentPartition)->size;
                    unallocatedPartitions.erase(next(currentPartition));
                    allPartitions.erase(next(currentPartition));
                }

                // Inserts the new unallocated partition into the unallocated partitions set
                unallocatedPartitions.insert(unallocatedPartitions.begin(), currentPartition);
            }

            // Removes all lookup values tied to the current tag from the partition lookup table (as they are now unallocated partitions and might've been merged)
            partitionLookupTable.erase(tag);
        }
    }

    /**
     * Function that grabs all the relevant information generated overtime by using the simulator and returns to
     * @note Implements code from memsim-w21 (https://gitlab.com/cpsc457/public/memsim-w21)
     * @return MemSimResult - Results
     */
    MemSimResult getStats() {
        // Checks to see if any partitions were created at all otherwise returns the default result values
        if (!unallocatedPartitions.empty()) {
            result.max_free_partition_address = (*unallocatedPartitions.begin())->address;
            result.max_free_partition_size = (*unallocatedPartitions.begin())->size;
        }

        // Returns the result back to the calling code
        return result;
    }
};

/**
 * Function that uses the provided page size and requests to simulate a memory partitioner and returns the results
 * @note Implements code from memsim-w21 (https://gitlab.com/cpsc457/public/memsim-w21)
 * @param page_size - Integer of the page size that will be used for all future partition allocation / deallocation actions
 * @param requests - Pointer to a vector consisting of Request that will indicate when to allocate / deallocate partitions and what their tag and size should be
 * @return MemSimResult - Results
 */
MemSimResult mem_sim(int64_t page_size, const std::vector<Request> &requests) {
    // Initializes the memory simulator using the specified page size
    Simulator memorySimulator(page_size);

    // Loops through all the requests and calls the appropriate functions for each request
    for (const auto &currentRequest : requests) {
        // Checks to see if the tag of the current request is negative (indicates a deallocation request) otherwise runs the allocation code
        if (currentRequest.tag < 0)
            memorySimulator.deallocate(-currentRequest.tag);
        else
            memorySimulator.allocate(currentRequest.tag, currentRequest.size);
    }

    // Returns teh results to the calling code
    return memorySimulator.getStats();
}
