#include "detectPrimes.h"
#include <pthread.h>
#include <cmath>
#include <cstdio>

using namespace std;

// Initialize the vector that will store all the prime numbers that were found from the input
vector<int64_t> results;

// Stores the index of next number to check
int nextNumberIndex = 0;

// Stores the number that is currently being checked
int64_t currentNumber = 0;

// Boolean that will control when to stop all the threads from further checks
bool shouldStopChecks = false;

// Initialize a thread barrier
pthread_barrier_t threadBarrier;

// Custom data struct that will store the parameters used for each thread's work
struct threadParameters {
    int threadNumber;
    vector<int64_t> numbersVector;
};

void *parallelWork(void *input) {
    printf("ID: %d\n", ((threadParameters *) input)->threadNumber);
    exit(0);

//    // try to divide n by every number 5 .. sqrt(n)
//    int64_t i = 5;
//    int64_t max = sqrt(currentNumber);
//    while (i <= max) {
//        if (currentNumber % i == 0) return false;
//        if (currentNumber % (i + 2) == 0) return false;
//        i += 6;
//    }
//    // didn't find any divisors, so it must be a prime
//    return true;
}

void *threadWork(void *input) {
    // Keeps running the thread until the global stop flag has been set to true
    while (!shouldStopChecks) {
        // Stops all threads here unless its selected to be the serial thread which can proceed
        if (pthread_barrier_wait(&threadBarrier) == PTHREAD_BARRIER_SERIAL_THREAD) {
            // Checks to see if the entire input has been checked
            if (nextNumberIndex < ((threadParameters *) input)->numbersVector.size()) {
                // Grabs the next number in the input and increments the index to point to the next one for the next iteration
                currentNumber = ((threadParameters *) input)->numbersVector[nextNumberIndex];
                nextNumberIndex++;

                // Performs all the trivial checks and adds the number to the results if it is prime and if its not then skips further checks
                if (currentNumber < 2) continue;
                if (currentNumber <= 3) {
                    results.push_back(currentNumber);
                    continue;
                }
                if (currentNumber % 2 == 0) continue;
                if (currentNumber % 3 == 0) continue;

                // TODO Divide work to all threads
            } else {
                // Sets the global flag to indicate the threads to stop if the entire input has been checked
                shouldStopChecks = true;
            }
        }

        // All threads wait here until the serial thread catches up (grabs the next number to check)
        pthread_barrier_wait(&threadBarrier);

        // If the serial thread had set the global stop flag indicating that the entire input has been checked then stops the thread
        if (shouldStopChecks)
            break;

        // TODO Do parallel work
        parallelWork(input);

        // TODO Do serial work of combining all the work and update global result
        if (pthread_barrier_wait(&threadBarrier) == PTHREAD_BARRIER_SERIAL_THREAD) {

        }
        pthread_barrier_wait(&threadBarrier);
    }
    return nullptr;
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
    // Checks to see how many threads were requested and runs the single threaded or multi-threaded code as necessary
    if (n_threads == 1) {
        // Runs the single threaded provided code
        for (auto num : nums) {
            if (is_prime(num))
                results.push_back(num);
        }
    } else {
        // Creates an array of threads based on how many were requested to be used
        pthread_t threadsArray[n_threads];

        // Initializes the thread barrier to handle the number of threads that were requested to be used
        pthread_barrier_init(&threadBarrier, NULL, n_threads);

        // Loop to assign work to each of the threads
        for (int currentThreadIndex = 0; currentThreadIndex < n_threads; currentThreadIndex++) {
            pthread_create(&threadsArray[currentThreadIndex], nullptr, threadWork,
                           (void *) new threadParameters{currentThreadIndex, nums});
        }

        // Loop to garbage collect all the threads
        for (int currentThreadIndex = 0; currentThreadIndex < n_threads; currentThreadIndex++) {
            // Closes the thread
            pthread_join(threadsArray[currentThreadIndex], nullptr);
        }

        // Garbage collects the thread barrier
        pthread_barrier_destroy(&threadBarrier);
    }

    // Returns the populated result vector
    return results;
}
