#include "scheduler.h"
#include "common.h"
#include <queue>
#include <cmath>
#include <iostream>

using namespace std;

/**
 * Function that uses the provided processes vector and schedules them in a round robin manner where quantum is the slice length
 * @note Implements code from scheduler (https://gitlab.com/cpsc457/public/scheduler) and Gabriela Wcislo's fcfsSimulationLoop.cpp from jun2_code
 * @param quantum - Time slice length before which a process is context switched off the CPU
 * @param max_seq_len - Number of sequences to return in the generated schedule (cuts off anything that occurs after this number)
 * @param processes - Pointer to a vector consisting of Process objects that will be scheduled
 * @param seq - Pointer to an integer vector that will contain the order that processes are scheduled in based on Process ID
 */
void simulate_rr(
        int64_t quantum,
        int64_t max_seq_len,
        std::vector<Process> &processes,
        std::vector<int> &seq
) {
    // Clears any pre-existing entries in the schedule
    seq.clear();

    // Stores the process id of the process currently on the CPU (-1 = idle)
    int currentProcessID = -1;

    // Stores the remaining time left for the process to complete
    int64_t remainingTime = 0;

    // Creates a queue that will store each Process as a pair consisting of the process's id and remaining time
    queue<pair<int, int64_t>> readyQueue;

    // Stores the current time in the schedule
    int64_t currentTime = 0;

    // Stores how many processes have arrived
    int processesArrived = 0;

    // Stores how many processes are remaining
    int processesRemaining = int(processes.size());

    // Stores the time that the current process time spent on the CPU
    int64_t timeOnCPU = 0;

    // Stores a boolean to keep track of when a time jump occurs to prevent incrementing the time for that iteration
    bool jumpOccurred = false;

    // Loops until all processes have been complete (scheduled)
    while (true) {
        // If there are no processes remaining then breaks the loop as we are done
        if (processesRemaining == 0) break;

        // Checks to see if current process on the CPU is done and replaces it with an idle process if it is
        if (currentProcessID > -1 && remainingTime == 0) {
            processes[currentProcessID].finish_time = currentTime;
            currentProcessID = -1;
            processesRemaining--;
            timeOnCPU = 0;
            continue;
        }

        // Checks to see if the current process on the CPU has exceeded the time slice allowed per process and adds it back to the ready queue if there is another process waiting (otherwise the current process is allowed to continue)
        if (currentProcessID > -1 && timeOnCPU >= quantum && !readyQueue.empty()) {
            readyQueue.push(make_pair(currentProcessID, remainingTime));
            currentProcessID = -1;
            timeOnCPU = 0;
            continue;
        }

        // Checks to see if there is an incoming process and adds it to the ready queue if it has arrived
        if (!processes.empty() && processes[processesArrived].arrival_time == currentTime) {
            if (currentProcessID > -1 && timeOnCPU >= quantum) {
                readyQueue.push(make_pair(currentProcessID, remainingTime));
                currentProcessID = -1;
                timeOnCPU = 0;
            }
            readyQueue.push(make_pair(processesArrived, processes[processesArrived].burst));
            processesArrived++;
            continue;
        }

        // Checks to see if the CPU is idle and if there is a process at the front of the ready queue (has arrived) then allows the ready process to use the CPU
        if (currentProcessID == -1 && !readyQueue.empty()) {
            currentProcessID = readyQueue.front().first;
            remainingTime = readyQueue.front().second;
            readyQueue.pop();
            timeOnCPU = 0;
            if (processes[currentProcessID].start_time == -1)
                processes[currentProcessID].start_time = currentTime;
            continue;
        }

        // If the current process is the last remaining process then skips to the process's end time (implements optimization hint 2)
        if (currentProcessID > -1 && processesRemaining == 1) {
            currentTime += remainingTime;
            timeOnCPU += remainingTime;
            remainingTime = 0;
            if ((seq.empty() || seq.back() != currentProcessID) && int64_t(seq.size()) < max_seq_len)
                seq.push_back(currentProcessID);
            jumpOccurred = true;
            continue;
        }

        // If the CPU is currently idle and there will be a process arriving in the future then skips to its arrival time (implements optimization hint 3)
        if (currentProcessID == -1 && readyQueue.empty()) {
            currentTime = processes[processesArrived].arrival_time;
            if ((seq.empty() || seq.back() != currentProcessID) && int64_t(seq.size()) < max_seq_len)
                seq.push_back(currentProcessID);
            jumpOccurred = true;
            continue;
        }

        // If there is no processes waiting in the ready queue then skips either to the arrival time of the next process or the end time of the current process based on the smaller value (implements optimization hint 4)
        if (currentProcessID > -1 && readyQueue.empty()) {
            if (abs(currentTime - processes[processesArrived].arrival_time) <= remainingTime) {
                remainingTime -= abs(currentTime - processes[processesArrived].arrival_time);
                timeOnCPU += abs(currentTime - processes[processesArrived].arrival_time);
                currentTime += abs(currentTime - processes[processesArrived].arrival_time);
                if ((seq.empty() || seq.back() != currentProcessID) && int64_t(seq.size()) < max_seq_len)
                    seq.push_back(currentProcessID);
                jumpOccurred = true;
                continue;
            } else if (abs(currentTime - processes[processesArrived].arrival_time) > remainingTime) {
                currentTime += remainingTime;
                timeOnCPU += remainingTime;
                remainingTime = 0;
                if ((seq.empty() || seq.back() != currentProcessID) && int64_t(seq.size()) < max_seq_len)
                    seq.push_back(currentProcessID);
                jumpOccurred = true;
                continue;
            }
        }

        // If the current process's remaining time is less than a quantum - time elapsed on CPU, then skips to either the arrival time of the next process or the end of the slice based on the smaller value (implements optimization hint 1)
        if (currentProcessID > -1 && remainingTime <= abs(timeOnCPU - quantum)) {
            if (abs(currentTime - processes[processesArrived].arrival_time) <= abs(timeOnCPU - quantum)) {
                remainingTime -= abs(currentTime - processes[processesArrived].arrival_time);
                timeOnCPU += abs(currentTime - processes[processesArrived].arrival_time);
                currentTime += abs(currentTime - processes[processesArrived].arrival_time);
                if ((seq.empty() || seq.back() != currentProcessID) && int64_t(seq.size()) < max_seq_len)
                    seq.push_back(currentProcessID);
                jumpOccurred = true;
                continue;
            } else if (abs(currentTime - processes[processesArrived].arrival_time) > abs(timeOnCPU - quantum)) {
                currentTime += abs(timeOnCPU - quantum);
                timeOnCPU += abs(timeOnCPU - quantum);
                remainingTime = abs(timeOnCPU - quantum);
                if ((seq.empty() || seq.back() != currentProcessID) && int64_t(seq.size()) < max_seq_len)
                    seq.push_back(currentProcessID);
                jumpOccurred = true;
                continue;
            }
        }

        // Adds to the schedule sequence if necessary (is a condensed schedule that doesn't exceed the length specified by the calling code)
        if ((seq.empty() || seq.back() != currentProcessID) && int64_t(seq.size()) < max_seq_len)
            seq.push_back(currentProcessID);

        // Print the current item on CPU
        if (currentProcessID >= 0) cout << "T" << currentTime << ":\t P" << currentProcessID << endl;
        else cout << "T" << currentTime << ":\t Idle" << endl;

        // Performs work (decrements the process's remaining time) and increments the time spent on the CPU if a jump hasn't occurred this run
        if (!jumpOccurred) {
            if (remainingTime > 0) remainingTime--;
            currentTime++;
            timeOnCPU++;
        } else
            jumpOccurred = false;
    }
}
