#include "detectPrimes.h"
#include <pthread.h>
#include <cmath>
#include <atomic>
#include <unordered_map>

using namespace std;

// Initialize the vector that will store all the prime numbers that were found from the input
vector<int64_t> results;

// Stores the index of the next number to check
ulong nextNumberIndex = 0;

// Stores the number that is currently being checked
int64_t currentNumber = 0;

// Boolean that will control when to stop all the threads from further checks
atomic_bool shouldStopChecks;

// Initialize an integer to store the result from the current number's check (0 = not determined yet, -1 = not prime, 1 = prime)
atomic_int currentNumberResult;

// Initialize an unordered map that will store the results from each of the numbers checked (to skip having to check duplicate numbers)
unordered_map<int64_t, int> checkedNumbers;

// Initialize a thread barrier
pthread_barrier_t threadBarrier;

// Custom data struct that will store the parameters used for each thread's work
struct threadParameters {
    int threadNumber;
    int totalThreads;
    vector<int64_t> numbersVector;
};

/**
 * Function that will be used by threads to perform their work
 * @param input - Pointer that will contain the threadParameters struct to pass in input to the thread
 */
void *threadWork(void *input) {
    // Keeps running the thread until the global stop flag has been set to true
    while (!shouldStopChecks) {
        // Stops all threads here unless its selected to be the serial thread which can proceed
        if (pthread_barrier_wait(&threadBarrier) == PTHREAD_BARRIER_SERIAL_THREAD) {
            // Checks to see if the entire input has been checked
            if (nextNumberIndex < ((threadParameters *) input)->numbersVector.size()) {
                // Sets the result value of the number that will be checked back to its default value (resets pre-existing data)
                currentNumberResult = 0;

                // Grabs the next number in the input and increments the index to point to the next one for the next iteration
                currentNumber = ((threadParameters *) input)->numbersVector[nextNumberIndex];
                nextNumberIndex++;

                // Checks to see if the current number has already been checked and reuses the same result if it has
                if (checkedNumbers[currentNumber] != 0)
                    currentNumberResult = checkedNumbers[currentNumber];
                else {
                    // Performs all the trivial checks and updates the results relating to the current number if any were conclusive
                    if (currentNumber < 2)
                        currentNumberResult = -1;
                    else if (currentNumber <= 3)
                        currentNumberResult = 1;
                    else if (currentNumber % 2 == 0)
                        currentNumberResult = -1;
                    else if (currentNumber % 3 == 0)
                        currentNumberResult = -1;
                }
            } else
                // Sets the global flag to indicate the threads to stop if the entire input has been checked
                shouldStopChecks = true;
        }

        // All threads wait here until the serial thread catches up (grabs the next number to check)
        pthread_barrier_wait(&threadBarrier);

        // If the serial thread had set the global stop flag indicating that the entire input has been checked then stops the thread
        if (shouldStopChecks)
            break;

        // Stores the number of checks that each thread has to perform based on the number of threads we have available
        int64_t checksPerThread =
                5 + ((sqrt(currentNumber) / (6 * (((threadParameters *) input)->totalThreads)))) * 6;

        // Initialize variables that will store the start and end values for the current thread's checks
        int64_t start;
        int64_t end;

        // If the current thread is the first thread then sets its starting value to 5
        if ((((threadParameters *) input)->threadNumber) == 0) {
            start = 5;
            end = checksPerThread;
        } else {
            // Sets the starting and ending values based on the current and next thread's ids
            start = checksPerThread * (((threadParameters *) input)->threadNumber);
            end = checksPerThread * ((((threadParameters *) input)->threadNumber) + 1);

            // Decrements the lower bound to make it compliant with the algorithm (makes sure we are starting from a valid point and performing the appropriate checks)
            while (((start - 5) % 6) != 0)
                start--;
        }

        // Caps the last thread's ending value if necessary by the algorithm specified bound
        if (end > sqrt(currentNumber))
            end = sqrt(currentNumber);

        // Loops from the current thread's starting to ending values and performs the primality checks while also checking if the result has not been found by another thread (stops further checks if it has been)
        while (start <= end && currentNumberResult == 0) {
            // Updates the value storing the current number's primality to indicate that it is not a prime if the check succeeds
            if (currentNumber % start == 0)
                currentNumberResult = -1;
                // Updates the value storing the current number's primality to indicate that it is not a prime if the check succeeds
            else if (currentNumber % (start + 2) == 0)
                currentNumberResult = -1;

            // Increments the starting value to the next number that needs to be checked
            start += 6;
        }

        // Stops all threads here unless its selected to be the serial thread which can proceed
        if (pthread_barrier_wait(&threadBarrier) == PTHREAD_BARRIER_SERIAL_THREAD) {
            // Adds the number to the results vector if it was determined to be a prime number
            if (currentNumberResult != -1)
                results.push_back(currentNumber);

            // Adds the number and its result to the unordered map to prevent checking a duplicate occurrence
            checkedNumbers[currentNumber] = currentNumberResult;
        }

        // All threads wait here until the serial thread catches up (adds the number to the vector if its a prime number)
        pthread_barrier_wait(&threadBarrier);
    }
    return nullptr;
}

/**
 * Function that uses the provided n (number) and checks its primality
 * @note Implements code from detectPrimes (https://gitlab.com/cpsc457/public/detectPrimes)
 * @param n - Number to check the primality of
 * @return bool - Boolean of whether or not the passed in number is prime (False = not prime, True = prime)
 */
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

/**
 * Function that uses the provided nums (vector of numbers to check) and n_threads (number of threads) to check their primality
 * @note Implements code from detectPrimes (https://gitlab.com/cpsc457/public/detectPrimes) and Gabriela Wcislo's simple_pthread.cpp and simple_barrier from w3d2_code
 * @param num - Vector of 64 bit wide integers that will have their primality checked for
 * @param n_threads - Max number of threads that can be utilized
 * @return vector - Vector of 64 bit wide integers that are prime numbers from the passed in list of numbers
 */
std::vector<int64_t> detect_primes(const std::vector<int64_t> &nums, int n_threads) {
    // Checks to see how many threads were requested and runs the single threaded or multi-threaded code as necessary
    if (n_threads == 1) {
        // Runs the single threaded provided code
        for (auto num : nums)
            if (is_prime(num))
                results.push_back(num);
    } else {
        // Creates an array of threads based on how many were requested to be used
        pthread_t threadsArray[n_threads];

        // Initializes the thread barrier to handle the number of threads that were requested to be used
        pthread_barrier_init(&threadBarrier, nullptr, n_threads);

        // Sets the boolean that will control when to stop all the threads from further checks to false
        shouldStopChecks = false;

        // Loop to assign work to each of the threads
        for (int currentThreadIndex = 0; currentThreadIndex < n_threads; currentThreadIndex++)
            pthread_create(&threadsArray[currentThreadIndex], nullptr, threadWork,
                           (void *) new threadParameters{currentThreadIndex, n_threads, nums});

        // Loop to garbage collect all the threads
        for (int currentThreadIndex = 0; currentThreadIndex < n_threads; currentThreadIndex++)
            // Closes the thread
            pthread_join(threadsArray[currentThreadIndex], nullptr);

        // Garbage collects the thread barrier
        pthread_barrier_destroy(&threadBarrier);
    }

    // Returns the populated result vector
    return results;
}
