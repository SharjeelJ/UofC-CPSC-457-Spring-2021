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
    double startX;
    double endX;
};

/**
 * Function that will be used by threads to perform their work
 * @param input - Pointer that will contain the threadParameters struct to pass in input to the thread
 * @return uint64_t - Pointer to an unsigned 64 bit wide integer that will store the result from the thread's work
 */
void *threadWork(void *input) {
    // Initialize a local counter to that will store the result that will be returned
    uint64_t counter = 0;

    // Loop that runs within the start and end bounds passed in through the data struct
    for (double x = ((threadParameters *) input)->startX; x <= ((threadParameters *) input)->endX; x++)
        // Loop that runs radius + 1 number of times
        for (double y = 0; y <= radius; y++)
            // Checks to see if (x^2 + y^2) <= radius^2 and increments the local counter if it is
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

    // Stores the radius^2 value as a double (retains decimal places)
    radiusSquared = double(radius) * radius;

    // Initialize an unsigned 64 bit wide integer counter that will store the final result
    uint64_t resultCounter = 0;

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
        // Loop that runs radius number of times (x starts at 1 and ends at radius) so runs n(INSIDE) times
        for (double x = 1; x <= radius; x++)
            // Loop that runs radius + 1 number of times (starts at 0 and ends at radius) so runs OUTSIDE(n+1) times
            for (double y = 0; y <= radius; y++)
                // Checks to see if (x^2 + y^2) <= radius^2 and increments the resultCounter if it is so runs n(n+1) times
                if (x * x + y * y <= radiusSquared)
                    resultCounter++;
    } else {
        // Integers to store the current x bounds being worked on by the threads
        double startX = 1;
        double endX = workPerThread;

        // Loop to assign work to each of the threads
        for (int currentThreadIndex = 0; currentThreadIndex < threadsNeeded; currentThreadIndex++) {
            // Creates a thread based on the current x bounds that need to be worked on
            pthread_create(&threadsArray[currentThreadIndex], nullptr, threadWork,
                           (void *) new threadParameters{startX, endX});

            // Adjusts the x bounds in preparation of the next thread
            startX += workPerThread;
            endX += workPerThread;

            // If the x upper bound is greater than the radius then sets it to be the radius
            if (endX > radius)
                endX = radius;
        }

        // Loop to garbage collect all the threads
        for (int currentThreadIndex = 0; currentThreadIndex < threadsNeeded; currentThreadIndex++) {
            // Creates a pointer that will store the thread's counter result
            void *threadResult = 0;

            // Closes the thread and stores its returned result
            pthread_join(threadsArray[currentThreadIndex], &threadResult);

            // Increments the main result counter with the thread's localized counter (merges the results from the threads)
            resultCounter += reinterpret_cast<uint64_t>(threadResult);
        }
    }

    // Returns 4 times the value of the result counter as there are 4 quadrants when dealing with a grid (and we only solved for one quadrant as the rest are similar)
    return resultCounter * 4 + 1;
}
