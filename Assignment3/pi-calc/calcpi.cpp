// ======================================================================
// You must modify this file and then submit it for grading to D2L.
// ======================================================================
//
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

#include "calcpi.h"
#include <pthread.h>
#include <stdio.h>

using namespace std;

// Initialize an unsigned 64 bit wide integer counter
uint64_t counter = 0;

// Initialize an integer that will store the r value
int radius = 0;

// Initialize a double that will store the r^2 value as a double (retains decimal places)
double radiusSquared;

// Custom data structure that will store the parameters for each thread
struct threadParameters {
    int x;
    int y;
};

// Creates a thread with the input being a pointer to the passed in data
void *threadedWork(void *input) {
    // Prints out the data that was passed in through the custom data struct
    printf("Thread Input: (%d,%d)\n", ((threadParameters *) input)->x, ((threadParameters *) input)->y);

    // Summation 1 (Loop that increments x by 1, r number of times (x starts at 1 and ends at r)
    for (double x = 1; x <= radius; x++) //
        // Summation 2 (Loop that increments y by 1, r number of times (starts at 0 and ends at r)
        for (double y = 0; y <= radius; y++)
            // Checks to see if (x^2 + y^2) <= r^2 and increments the counter if it is
            if (x * x + y * y <= radiusSquared)
                counter++;

    return nullptr;
}

uint64_t count_pixels(int r, int n_threads) {
    // Stores the number of loop iterations (checks that will need to be performed) based on the provided r value based on (sum (sum 1, y = 0 to r), x = 1 to r)
    int totalChecksNeeded = r * (r + 1);

    // Initialize integers that will store the number of threads that will be used and how much work per thread will be assigned
    int threadsNeeded;
    int workPerThread;

    // Computes the number of threads that will be used and the work per thread based on if more threads are allowed than work available
    if (totalChecksNeeded <= n_threads) {
        threadsNeeded = totalChecksNeeded;
        workPerThread = 1;
    }
        // Computes the number of threads that will be used and the work per thread based on if less threads are allowed than work available
    else {
        threadsNeeded = n_threads;
        workPerThread = totalChecksNeeded / n_threads;
    }

    // Creates an array of n threadsArray
    pthread_t threadsArray[threadsNeeded];

    // Prints out the data from the above calculations
    printf("Checks needed: %d * 4 = %d\n", totalChecksNeeded, totalChecksNeeded * 4);
    printf("Threads: %d\nChecks per thread: %d (%lf)\n\n", threadsNeeded, workPerThread,
           double(totalChecksNeeded) / n_threads);

    // Stores the passed in r value
    radius = r;

    // Stores the r^2 value as a double (retains decimal places)
    radiusSquared = double(r) * r;

    // If the code was specified to run on a single thread then runs the provided code as is
    if (n_threads == 1) {
        // Summation 1 (Loop that increments x by 1, r number of times (x starts at 1 and ends at r)
        for (double x = 1; x <= r; x++) //
            // Summation 2 (Loop that increments y by 1, r number of times (starts at 0 and ends at r)
            for (double y = 0; y <= r; y++)
                // Checks to see if (x^2 + y^2) <= r^2 and increments the counter if it is
                if (x * x + y * y <= radiusSquared)
                    counter++;
    } else {
        pthread_create(&threadsArray[0], NULL, threadedWork, (void *) new threadParameters{1, 1});
        pthread_create(&threadsArray[1], NULL, threadedWork, (void *) new threadParameters{1, 2});
        pthread_join(threadsArray[0], NULL);
        pthread_join(threadsArray[1], NULL);
        printf("\n");
    }

    // Returns 4 times the value of the counter as there are 4 quadrants when dealing with a grid (and we only solved for one quadrant as the rest are similar)
    return counter * 4 + 1;
}
