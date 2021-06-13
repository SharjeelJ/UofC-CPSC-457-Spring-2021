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

    int processOnCPU = -1; // Note processes ids start at 0, let -1 denote "idle"
    int64_t burstRemaining = 0;

    queue<pair<int, int64_t>> readyQueue; // elements are pairs: (process ID, burst remaining)
    int64_t currentTime = 0;
    int jobsArrived = 0;
    int jobsRemaining = processes.size();

    int64_t timeOnCPU = 0;

    while (1) {
        if (jobsRemaining == 0) break;

        // Check: if process on CPU is done
        if (processOnCPU > -1 && burstRemaining == 0) {
            processes[processOnCPU].finish_time = currentTime;
            processOnCPU = -1;
            jobsRemaining--;
            timeOnCPU = 0;
            continue;
        }

        if (processOnCPU > -1 && timeOnCPU >= quantum && !readyQueue.empty()) {
            readyQueue.push(make_pair(processOnCPU, burstRemaining));
            processOnCPU = -1;
            timeOnCPU = 0;
            continue;
        }

        // Check: if a new process is arriving
        if (processes.size() > 0 && processes[jobsArrived].arrival_time == currentTime) {
            if (processOnCPU > -1 && timeOnCPU >= quantum) {
                readyQueue.push(make_pair(processOnCPU, burstRemaining));
                processOnCPU = -1;
                timeOnCPU = 0;
            }
            readyQueue.push(make_pair(jobsArrived, processes[jobsArrived].burst));
            jobsArrived++;
            continue;
        }

        // Check: if CPU is idle and ready queue is not empty
        if (processOnCPU == -1 && !readyQueue.empty()) {
            processOnCPU = readyQueue.front().first;
            burstRemaining = readyQueue.front().second;
            readyQueue.pop();
            timeOnCPU = 0;
            if (processes[processOnCPU].start_time == -1)
                processes[processOnCPU].start_time = currentTime;
            continue;
        }

        // Update the execution order if needed
        if ((seq.empty() || seq.back() != processOnCPU) && seq.size() < max_seq_len) {
            seq.push_back(processOnCPU);
        }

        // Print the current item on CPU
        if (processOnCPU >= 0) cout << "T" << currentTime << ":\t P" << processOnCPU << endl;
        else cout << "T" << currentTime << ":\t Idle" << endl;

        // Perform a CPU burst then increment the time
        if (burstRemaining > 0) burstRemaining--;
        currentTime++;
        timeOnCPU++;
    }
}
