#include "detectPrimes.h"
#include <pthread.h>
#include <cmath>
#include <cstdio>
#include <atomic>

using namespace std;

// Initialize the vector that will store all the prime numbers that were found from the input
vector<int64_t> results;

// Stores the index of next number to check
int nextNumberIndex = 0;

// Stores the number that is currently being checked
int64_t currentNumber = 0;

// Boolean that will control when to stop all the threads from further checks
atomic_bool shouldStopChecks;

// Initialize an integer to store the result from the current number's check
atomic_int currentNumberResult;

// Initialize a thread barrier
pthread_barrier_t threadBarrier;

// Custom data struct that will store the parameters used for each thread's work
struct threadParameters {
    int threadNumber;
    int totalThreads;
    vector<int64_t> numbersVector;
};

void *threadWork(void *input) {
    // Keeps running the thread until the global stop flag has been set to true
    while (!shouldStopChecks) {
        // Stops all threads here unless its selected to be the serial thread which can proceed
        if (pthread_barrier_wait(&threadBarrier) == PTHREAD_BARRIER_SERIAL_THREAD) {
            // Checks to see if the entire input has been checked
            if (nextNumberIndex < ((threadParameters *) input)->numbersVector.size()) {
                // Sets the result of the number that will be checked back to its default value
                currentNumberResult = -1;

                // Grabs the next number in the input and increments the index to point to the next one for the next iteration
                currentNumber = ((threadParameters *) input)->numbersVector[nextNumberIndex];
                nextNumberIndex++;

                // Performs all the trivial checks
                if (currentNumber < 2)
                    currentNumberResult = 0;
                else if (currentNumber <= 3)
                    currentNumberResult = 1;
                else if (currentNumber % 2 == 0)
                    currentNumberResult = 0;
                else if (currentNumber % 3 == 0)
                    currentNumberResult = 0;
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

        int64_t checksPerThread =
                5 + ((sqrt(currentNumber) / (6 * (((threadParameters *) input)->totalThreads)))) * 6;
        int64_t start = 5;
        int64_t end = sqrt(currentNumber);

        if ((((threadParameters *) input)->threadNumber) == 0) {
            start = 5;
            end = checksPerThread;
        } else {
            start = checksPerThread * (((threadParameters *) input)->threadNumber);
            end = checksPerThread * ((((threadParameters *) input)->threadNumber) + 1);

            while (((start - 5) % 6) != 0) {
                start--;
            }
        }

        if (end > sqrt(currentNumber)) {
            end = sqrt(currentNumber);
        }

        if (start == 0)
            start = 5;

//        printf("\nTotal Checks: %ld\n", totalChecks);
        printf("Per Thread: %ld\n", checksPerThread);
        printf("START: %ld END: %ld\n\n", start, end);
//        printf("TEST: %ld", test);

        int64_t i = start;
        int64_t max = end;
        while (i <= max && currentNumberResult == -1) {
            if (currentNumber % i == 0) {
                currentNumberResult = 0;
            } else if (currentNumber % (i + 2) == 0) {
                currentNumberResult = 0;
            }
            i += 6;
        }

        // Stops all threads here unless its selected to be the serial thread which can proceed
        if (pthread_barrier_wait(&threadBarrier) == PTHREAD_BARRIER_SERIAL_THREAD) {
            // Adds the number to the results vector if it was determined to be a prime number
            if (currentNumberResult != 0) {
                results.push_back(currentNumber);
            }
        }

        // All threads wait here until the serial thread catches up (adds the number to the vector if its a prime number)
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

        // Sets the boolean that will control when to stop all the threads from further checks to false
        shouldStopChecks = false;

        // Loop to assign work to each of the threads
        for (int currentThreadIndex = 0; currentThreadIndex < n_threads; currentThreadIndex++) {
            pthread_create(&threadsArray[currentThreadIndex], nullptr, threadWork,
                           (void *) new threadParameters{currentThreadIndex, n_threads, nums});
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
