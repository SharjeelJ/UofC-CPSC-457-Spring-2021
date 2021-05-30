#include "detectPrimes.h"
#include <pthread.h>
#include <cmath>
#include <cstdio>
#include <cstdlib>

using namespace std;

// Custom data struct that will store the parameters used for each thread's work
struct threadParameters {
    int64_t inputNumber;
};

void *threadedWork(void *input) {
    int64_t n = ((threadParameters *) input)->inputNumber;

    // handle trivial cases
    if (n < 2) return (void *) false;
    if (n <= 3) return (void *) true; // 2 and 3 are primes
    if (n % 2 == 0) return (void *) false; // handle multiples of 2
    if (n % 3 == 0) return (void *) false; // handle multiples of 3
    // try to divide n by every number 5 .. sqrt(n)
    int64_t i = 5;
    int64_t max = sqrt(n);
    while (i <= max) {
        if (n % i == 0) return (void *) false;
        if (n % (i + 2) == 0) return (void *) false;
        i += 6;
    }

    // didn't find any divisors, so it must be a prime
    return (void *) true;
}

// returns true if n is prime, otherwise returns false
// -----------------------------------------------------------------------------
// to get full credit for this assignment, you will need to adjust or even
// re-write the code in this function to make it multithreaded.
static bool is_prime(int64_t n) {
    // handle trivial cases
    if (n < 2) return false;
    if (n <= 3) return true; // 2 and 3 are primes
    if (n % 2 == 0) return false; // handle multiples of 2
    if (n % 3 == 0) return false; // handle multiples of 3
    // try to divide n by every number 5 .. sqrt(n)
    int64_t i = 5;
    int64_t max = sqrt(n);
    while (i <= max) {
        if (n % i == 0) return false;
        if (n % (i + 2) == 0) return false;
        i += 6;
    }
    // didn't find any divisors, so it must be a prime
    return true;
}

// This function takes a list of numbers in nums[] and returns only numbers that
// are primes.
//
// The parameter n_threads indicates how many threads should be created to speed
// up the computation.
// -----------------------------------------------------------------------------
// You will most likely need to re-implement this function entirely.
// Note that the current implementation ignores n_threads. Your multithreaded
// implementation must use this parameter.
vector<int64_t> detect_primes(const vector<int64_t> &nums, int n_threads) {
    vector<int64_t> result;
    if (n_threads == 1) {
        for (auto num : nums) {
            if (is_prime(num))
                result.push_back(num);
        }
    } else {
        // Creates an array of threads based on the required number of threads that was computed
        pthread_t threadsArray[n_threads];

        // Creates a counter that will store the index of the next number to check
        int nextNumberToCheck = 0;

        // Loop that runs until all the numbers have been checked
        while (nextNumberToCheck < nums.size()) {
            // Stores the number of threads that are active (will be used to garbage collect later)
            int threadsActive = 0;

            // Loop to assign work to each of the threads
            for (int currentThreadIndex = 0;
                 currentThreadIndex < n_threads && nextNumberToCheck < nums.size(); currentThreadIndex++) {
                pthread_create(&threadsArray[currentThreadIndex], nullptr, threadedWork,
                               (void *) new threadParameters{(int64_t) nums[nextNumberToCheck]});

                // Increments the index of the next number to check
                nextNumberToCheck++;

                // Increments the counter keeping track of threads that were assigned work
                threadsActive++;
            }

            // Loop to garbage collect all the threads
            for (int currentThreadIndex = 0; currentThreadIndex < threadsActive; currentThreadIndex++) {
                // Creates a pointer that will store the resulting boolean information from the thread
                void *threadResult = nullptr;

                // Closes the thread and stores its returned result
                pthread_join(threadsArray[currentThreadIndex], &threadResult);

//                printf("%d\n", bool(threadResult));

                // Adds the number the thread worked on to the result vector if it was a prime number
                if (bool(threadResult))
                    result.push_back(nums[nextNumberToCheck - threadsActive + currentThreadIndex]);
            }
        }
    }
    return result;
}
