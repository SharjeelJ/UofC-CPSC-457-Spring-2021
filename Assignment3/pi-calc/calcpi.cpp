// ======================================================================
// You must modify this file and then submit it for grading to D2L.
// ======================================================================


#include "calcpi.h"
#include <pthread.h>
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

/**
 * Function that will be used by threads to perform their work
 * @param input - Pointer that will contain the threadedParameters struct to pass in input to the thread
 * @return uint64_t - Pointer to an unsigned 64 bit wide integer that will store the result from the thread's work
 */
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

/**
 * Function that uses the provided r (radius) and n_threads (number of threads) to count the number of pixels the area of a circle would encompass (implements https://en.wikipedia.org/wiki/Approximations_of_π#Summing_a_circle's_area )
 * @note Implements code from pi-calc (https://gitlab.com/cpsc457/public/pi-calc) and Gabriela Wcislo's simple_pthread.cpp and thread_sum.cpp from w3d2_code
 * @param r - Radius of the circle
 * @param n_threads - Max number of threads that can be utilized
 * @return uint64_t - Unsigned 64 bit wide integer that will store the number of pixels that were encompassed by the circle's area
 */
uint64_t count_pixels(int r, int n_threads) {
    // Returns 0 if r is 0
    if (r <= 0)
        return 0;

    // Stores the passed in r value
    radius = r;

    // Stores the r^2 value as a double (retains decimal places)
    radiusSquared = double(radius) * radius;

    // Initialize an unsigned 64 bit wide integer counter that will store the final result
    uint64_t counter = 0;

    // Initialize integers that will store the number of threads that will be used and how much work per thread will be assigned
    int threadsNeeded;
    uint64_t workPerThread;

    // Calculates the number of runs of the outer loop that each thread needs to do (using ceiling to round up so we have an excess of threads leftover in the best case)
    workPerThread = int(ceil((double(radius) / n_threads)));

    // Determines how many threads are necessary based on the workload we have in case we have more threads available than work that needs to be done (using floor to push off the excess work at the very end to a spare thread)
    threadsNeeded = int(floor((double(radius) / workPerThread)));

    // Determines if an excess thread will be needed based on if there is work left after all threads finish and will allocate it into the thread pool if required
    if ((radius - (workPerThread * threadsNeeded)) != 0)
        threadsNeeded++;

    // Creates an array of threads based on the required number of threads that was computed
    pthread_t threadsArray[threadsNeeded];

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
        for (int currentThreadIndex = 0; currentThreadIndex < threadsNeeded; currentThreadIndex++) {
            // Creates a thread based on the current x bounds that need to be worked on
            pthread_create(&threadsArray[currentThreadIndex], nullptr, threadedWork,
                           (void *) new threadParameters{startX, endX, 0});

            // Adjusts the x bounds in preparation of the next thread
            startX += workPerThread;
            endX += workPerThread;

            // If the x upper bound is greater than the radius then sets it to be the radius
            if (endX > radius)
                endX = radius;
        }

        // Loop to garbage collect all the threads
        for (int currentThreadIndex = 0; currentThreadIndex < threadsNeeded; currentThreadIndex++) {
            // Creates a pointer that will store the resulting local counter information from the thread
            void *threadResult = 0;

            // Closes the thread and adds its stores its returned result
            pthread_join(threadsArray[currentThreadIndex], &threadResult);

            // Increments the main counter with the thread's localized counter (merges the results from the threads)
            counter += reinterpret_cast<uint64_t>(threadResult);
        }
    }

    // Returns 4 times the value of the counter as there are 4 quadrants when dealing with a grid (and we only solved for one quadrant as the rest are similar)
    return counter * 4 + 1;
}
