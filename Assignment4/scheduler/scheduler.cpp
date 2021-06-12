#include "scheduler.h"
#include "common.h"
#include <queue>
#include <iostream>

using namespace std;

// this is the function you should edit
//
// runs Round-Robin scheduling simulator
// input:
//   quantum = time slice
//   max_seq_len = maximum length of the reported executing sequence
//   processes[] = list of process with populated IDs, arrival_times, and bursts
// output:
//   seq[] - will contain the execution sequence but trimmed to max_seq_len size
//         - idle CPU will be denoted by -1
//         - other entries will be from processes[].id
//         - sequence will be compressed, i.e. no repeated consecutive numbers
//   processes[]
//         - adjust finish_time and start_time for each process
//         - do not adjust other fields
//
void simulate_rr(
        int64_t quantum,
        int64_t max_seq_len,
        std::vector<Process> &processes,
        std::vector<int> &seq
) {
    // replace the wrong implementation below with your own!!!!
//    seq.clear();
//    for (auto &p : processes) {
//        p.finish_time = p.arrival_time + p.burst;
//        p.start_time = p.arrival_time;
//        seq.push_back(p.id);
//    }

    seq.clear();

    int processOnCPU = -1; // Note processes ids start at 0, let -1 denote "idle"
    int64_t burstRemaining = 0;

    queue<Process> readyQueue;
    int64_t currentTime = 0;
    int jobsArrived = 0;
    int jobsRemaining = processes.size();

    while (1) {
        if (jobsRemaining == 0) break;

        // Check: if process on CPU is done
        if (processOnCPU > -1 && burstRemaining == 0) {
            processOnCPU = -1;
            jobsRemaining--;
            continue;
        }

        // Check: if a new process is arriving
        if (processes.size() > 0 && processes[jobsArrived].arrival_time == currentTime) {
            readyQueue.push(processes[jobsArrived]);
            jobsArrived++;
            continue;
        }

        // Check: if CPU is idle and ready queue is not empty
        if (processOnCPU == -1 && !readyQueue.empty()) {
            processOnCPU = readyQueue.front().id;
            burstRemaining = readyQueue.front().burst;
            readyQueue.pop();
            continue;
        }

        // Update the execution order if needed
        if (seq.empty() || seq.back() != processOnCPU) {
            seq.push_back(processOnCPU);
        }

        // Print the current item on CPU
        if (processOnCPU >= 0) cout << "T" << currentTime << ":\t P" << processOnCPU << endl;
        else cout << "T" << currentTime << ":\t Idle" << std::endl;

        // Perform a CPU burst then increment the time
        if (burstRemaining > 0) burstRemaining--;
        currentTime++;
    }

    // Print the execution order
    bool printComma = false;
    std::cout << "Execution order: ";
    for (auto p : seq) {

        if (printComma) std::cout << ", ";
        else printComma = true;

        if (p >= 0) std::cout << "P" << p;
        else std::cout << "Idle";

    }
    std::cout << std::endl;
}

