#include "scheduler.h"
#include "common.h"
#include <queue>

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

    // Creates a current process object that will store the current process being worked on by the CPU
    Process currentProcess;

    // Creates a queue consisting of Process objects that will handle incoming and readied processes
    queue<Process> readyQueue;

    // Integer that will store the current time being worked on in the schedule
    int64_t currentTime = 0;

    // Integer to store the number of processes that have arrived
    int processesArrived = 0;

    // Stores the number of total processes that will need to occur (including ones that haven't arrived)
    int processesRemaining = processes.size();

    // Integer to keep track of how much time the current process has spent on the CPU
    int64_t timeSpentOnCPU = 0;

    // Vector to store the remaining time of each job in the queue
    vector<int> processesRemainingTime;
    processesRemainingTime.resize(processes.size());

    // Runs until all processes have been completed
    while (true) {
        // If there are no processes remaining then breaks the loop as we are done
        if (processesRemaining == 0) break;

        // Checks to see if current process on the CPU is done and replaces it with IDLE if it is
        if (currentProcess.id > -1 && processesRemainingTime[currentProcess.id] == 0) {
            processes[currentProcess.id].finish_time = currentTime;
            currentProcess = Process();
            timeSpentOnCPU = 0;
            processesRemaining--;
        }
            // Checks to see if the current process on the CPU has exceeded the time splice allowed per process and replaces it with the next process in the ready queue if there is one (otherwise the current process is allowed to continue)
        else if (currentProcess.id > -1 && timeSpentOnCPU >= quantum && !readyQueue.empty()) {
            readyQueue.push(currentProcess);
            currentProcess = readyQueue.front();
            timeSpentOnCPU = 0;
            // Sets the start time of the incoming process if it has never ran before
            if (processes[currentProcess.id].start_time == -1)
                processes[currentProcess.id].start_time = currentTime;
            readyQueue.pop();
        }
            // If the current process is the last remaining process then skips to end time - 1 (implements optimization hint 2)
        else if (currentProcess.id > -1 && readyQueue.empty() && processesRemaining == 1 &&
                 processesRemainingTime[currentProcess.id] > 1) {
            currentTime += processesRemainingTime[currentProcess.id] - 1;
            processesRemainingTime[currentProcess.id] = 1;
        }

        // Checks to see if there are any incoming processes and adds them to the ready queue if they have arrived
        if (processes.size() > 0 && processes[processesArrived].arrival_time == currentTime) {
            readyQueue.push(processes[processesArrived]);
            processesRemainingTime[processes[processesArrived].id] = processes[processesArrived].burst;
            processesArrived++;
        }

        // Checks to see if the CPU is idle and if there is a process at the front of the ready queue (has arrived) then allows the ready process to use the CPU
        if (currentProcess.id == -1 && !readyQueue.empty()) {
            currentProcess = readyQueue.front();
            // Sets the start time of the incoming process if it has never ran before
            if (processes[currentProcess.id].start_time == -1)
                processes[currentProcess.id].start_time = currentTime;
            timeSpentOnCPU = 0;
            readyQueue.pop();
        }
            // If the CPU is currently idle and there will be a process arriving in the future then skips to arrival time - 1 (implements optimization hint 3)
        else if (currentProcess.id == -1 && readyQueue.empty() && processesRemaining >= 1) {
            currentTime = processes[processesArrived].arrival_time - 1;
            seq.push_back(-1);
        }

        // TODO: Check Optimization hint 1
        if (currentProcess.id > -1 && processesRemainingTime[currentProcess.id] <= quantum && processesRemaining > 1) {
            if (currentTime + processesRemainingTime[currentProcess.id] <= processes[processesArrived].arrival_time &&
                readyQueue.empty()) {
                currentTime += processesRemainingTime[currentProcess.id] - 1;
                processesRemainingTime[currentProcess.id] -= processesRemainingTime[currentProcess.id] - 1;
                timeSpentOnCPU += processesRemainingTime[currentProcess.id] - 1;
                printf("HELLO1\n");
            } else if (currentTime + processesRemainingTime[currentProcess.id] <=
                       processes[processesArrived].arrival_time &&
                       currentTime + processesRemainingTime[currentProcess.id] <= quantum && !readyQueue.empty()) {
                currentTime += processesRemainingTime[currentProcess.id] - 1;
                processesRemainingTime[currentProcess.id] -= processesRemainingTime[currentProcess.id] - 1;
                timeSpentOnCPU += processesRemainingTime[currentProcess.id] - 1;
                printf("HELLO2\n");
            } else if (currentTime + processesRemainingTime[currentProcess.id] >=
                       processes[processesArrived].arrival_time &&
                       currentTime + processesRemainingTime[currentProcess.id] <= quantum && !readyQueue.empty()) {
                currentTime += processesRemainingTime[currentProcess.id] - processes[processesArrived].arrival_time - 1;
                processesRemainingTime[currentProcess.id] =
                        processesRemainingTime[currentProcess.id] - processes[processesArrived].arrival_time - 1;
                timeSpentOnCPU +=
                        processesRemainingTime[currentProcess.id] - processes[processesArrived].arrival_time - 1;
                printf("HELLO3\n");
            }
        }

        // Adds to the schedule sequence if necessary (is a condensed schedule that doesn't exceed the length specified by the calling code)
        if ((seq.empty() || seq.back() != currentProcess.id) && seq.size() < max_seq_len) {
            seq.push_back(currentProcess.id);
        }

        // Print the current item on CPU
        printf("%d) P:%d S:%d B:%d\n", currentTime, currentProcess.id, timeSpentOnCPU,
               processesRemainingTime[currentProcess.id]);
//        printf("%d\n%d\n%d\n%d\n\n", currentProcess.id, currentTime, readyQueue.size(), processesRemainingTime[currentProcess.id]);

        // Goes forward a CPU burst and increments the time spent
        if (processesRemainingTime[currentProcess.id] > 0) processesRemainingTime[currentProcess.id]--;
        currentTime++;
        timeSpentOnCPU++;
    }

    // Pops off the last entry from the sequence (-1)
    seq.pop_back();
}
