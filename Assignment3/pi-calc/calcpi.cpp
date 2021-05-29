// ======================================================================
// You must modify this file and then submit it for grading to D2L.
// ======================================================================


#include "calcpi.h"
#include <pthread.h>
#include <stdio.h>
#include <cmath>

using namespace std;

// Initialize an integer that will store the r value
int radius = 0;

// Initialize a double that will store the r^2 value as a double (retains decimal places)
double radiusSquared = 0;

// Custom data struct that will store the parameters used for each thread's work
struct threadParameters {
    int startX;
    int endX;
    uint64_t counter;
};

// Creates a thread with the input being a pointer to the passed in data
void *threadedWork(void *input) {
    // Initialize local counterparts to the information stored in the data struct (prevents aliasing)
    int startX = ((threadParameters *) input)->startX;
    int endX = ((threadParameters *) input)->endX;
    uint64_t counter = ((threadParameters *) input)->counter;

    // Loop that runs within the start and end bounds passed in through the data struct
    for (double x = startX; x <= endX; x++)
        // Loop that runs r + 1 number of times
        for (double y = 0; y <= radius; y++)
            // Checks to see if (x^2 + y^2) <= r^2 and increments the local counter if it is
            if (x * x + y * y <= radiusSquared)
                counter++;
    // Returns the local counter back to the calling code
    return (void *) counter;
}

// count_pi() calculates the number of pixels that fall into a circle
// using the algorithm explained here:
//
// https://en.wikipedia.org/wiki/Approximations_of_%CF%80
//
// count_pixels() takes 2 paramters:
//  r         =  the radius of the circle
//  n_threads =  the number of threads you should create
//
// Currently the function ignores the n_threads parameter. Your job is to
// parallelize the function so that it uses n_threads threads to do
// the computation.
uint64_t count_pixels(int r, int n_threads) {
    // Stores the passed in r value
    radius = r;

    // Stores the r^2 value as a double (retains decimal places)
    radiusSquared = double(radius) * radius;

    // Initialize an unsigned 64 bit wide integer counter that will store the final result
    uint64_t counter = 0;

    // Stores the number of loop iterations (checks that will need to be performed) based on the provided radius value based on (sum (sum 1, y = 0 to radius), x = 1 to radius)
    uint64_t totalChecksNeeded = radius * (radius + 1);

    // Initialize integers that will store the number of threads that will be used and how much work per thread will be assigned
    int threadsNeeded = 0;
    uint64_t workPerThread = 0;
    uint64_t lastThreadWork = 0;

    // Calculates the number of runs of the outer loop that each thread needs to do (using ceiling to round up so we have an excess of threads leftover in the best case)
    workPerThread = int(ceil((double(totalChecksNeeded) / (radius + 1)) / n_threads));

    // Determines how many threads are necessary based on the workload we have in case we have more threads available than work that needs to be done (using floor to push off the excess work at the very end to a spare thread)
    threadsNeeded = int(floor((double(totalChecksNeeded) / (radius + 1)) / workPerThread));

    // Determines how much left over work there will be that an excess thread will have to perform (if any)
    lastThreadWork = totalChecksNeeded - (threadsNeeded * workPerThread * (radius + 1));

    // Determines if an excess thread will be needed based on if there is work left after all threads finish and will allocate it into the thread pool if required
    if (lastThreadWork != 0)
        threadsNeeded++;

    // Creates an array of threads based on the required number of threads that was computed
    pthread_t threadsArray[threadsNeeded];

    // Prints out the data from the above calculations
    printf("\nChecks needed: %ld * 4 = %ld\n", totalChecksNeeded, totalChecksNeeded * 4);
    printf("Threads: %ld\n", threadsNeeded);
    printf("Checks per thread: %ld (%lf)\n", workPerThread,
           double(totalChecksNeeded) / (radius + 1) / n_threads);
    printf("Checks for last thread: %ld\n\n", lastThreadWork);

    // If the code was specified to run on a single thread then runs the provided code as is otherwise calls the multi-threaded code
    if (n_threads == 1) {
        // Loop that runs r number of times (x starts at 1 and ends at r) so runs n(INSIDE) times
        for (double x = 1; x <= radius; x++)
            // Loop that runs r + 1 number of times (starts at 0 and ends at r) so runs OUTSIDE(n+1) times
            for (double y = 0; y <= radius; y++)
                // Checks to see if (x^2 + y^2) <= r^2 and increments the counter if it is so runs n(n+1) times
                if (x * x + y * y <= radiusSquared)
                    counter++;
    } else {
        // Integers to store the current x bounds being worked on by the threads
        int startX = 1;
        int endX = workPerThread;

        // Loop to assign work to each of the threads
        for (pthread_t currentThread : threadsArray) {
            // Creates a thread based on the current x bounds that need to be worked on
            pthread_create(&currentThread, NULL, threadedWork, (void *) new threadParameters{startX, endX, 0});

            // Adjusts the x bounds in preparation of the next thread
            startX += workPerThread;
            endX += workPerThread;

            // If the x upper bound is greater than the radius then sets it to be the radius
            if (endX > radius)
                endX = radius;
        }
    }

    // Loop to garbage collect all the threads
    for (pthread_t currentThread : threadsArray) {
        // Creates a pointer that will store the resulting local counter information from the thread
        void *threadResult = 0;

        // Closes the thread and adds its stores its returned result
        pthread_join(currentThread, &threadResult);

        // Increments the main counter with the thread's localized counter (merges the results from the threads)
        counter += reinterpret_cast<uint64_t>(threadResult);
    }

    // TODO Remove below test code
    uint64_t checkResultsCounter = 0;
    for (double x = 1; x <= radius; x++) //
        for (double y = 0; y <= radius; y++)
            if (x * x + y * y <= radiusSquared)
                checkResultsCounter++;
    printf("Expected: %ld\n", checkResultsCounter);
    printf("Received: %ld\n\n", counter);

    // Returns 4 times the value of the counter as there are 4 quadrants when dealing with a grid (and we only solved for one quadrant as the rest are similar)
    return counter * 4 + 1;
}
