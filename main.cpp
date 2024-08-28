/*
How to run: 
gcc -o main main.cpp -lstdc++ -I/headers/
Made by: Jorjei Ngoche and NhatMinh Nguyen
Last edited: 5/13/24
*/
#include <random>
#include <iostream>
#include <queue>
#include <vector>
#include <iomanip>
#include "./headers/PCB.h"
#include "./headers/Event.h"
using namespace std;

const int PROCESS_COUNT = 75; // # of processes
const int PRIORITY_LEVELS = 1; // levels of priority
const int MIN_CPU_BURST_TIME = 10; // minimum burst time
const int MAX_CPU_BURST_TIME = 100; // maximum burst time
const int MIN_IO_BURST_TIME = 100;
const int MAX_IO_BURST_TIME = 150;

const int MAX_ARRIVAL = 1000; // arrival times randomized between 0 and MAX_ARRIVAL
const int MAX_SIM_TIME = 100000;

const int MAX_CPU_BURSTS = 7; // maximum number of cpu bursts a process can have
const int MIN_CPU_BURSTS = 3;
const int QUEUE_ITTR_LIMIT = 3; // number of times job can re-enter queue before priority gets lowered

int SERVER_POOL = 0; // SERVER_POOL is 0 when all servers are idle, equal to CPU_COUNT when all servers are busy
const int CPU_COUNT = 4; // number of servers
int cpuTimeUsed = 0; // amount of time CPUs have been running

const int quantum = 20; // quantum for RR
const double switchTime = 0.001; // amount of time taken to swap between processes
double accumulatedSwitchTime = 0; // amount of time taken for switching
int currentTime = 0; // the current system time

vector<Event> FEL; // future event list
PCB plist[PROCESS_COUNT]; // list of PCBs

const int QUEUE_COUNT = 2; // # of queues
queue<PCB*> qlist[QUEUE_COUNT]; // list of queues
const int queueTypeList[QUEUE_COUNT] = {4, 3}; // Type 1 for FCFS, Type 2 for Round-Robin. 
// Example: {1, 2, 2} is FCFS in the first queue, and RR in the second and third queues
// ATTENTION: the size of queueTypeList MUST equal QUEUE_COUNT


void create_PCBs(PCB x[PROCESS_COUNT]) {
    int randomPriority, randomBurstTime, randomArrivalTime, randomBurstCount;
    for(int i = 0; i < PROCESS_COUNT; i++) {
        //randomPriority = rand() % PRIORITY_LEVELS + 1;
        randomPriority = 1;
        vector<int> burstVector; // there should be a new burstVector for each new PCB
        randomBurstCount = (rand() % (MAX_CPU_BURSTS - MIN_CPU_BURSTS + 1) + MIN_CPU_BURSTS - 1) * 2 + 1; // should always have an odd number of CPU/IO bursts
        for(int j = 0; j < randomBurstCount; j++) {
            // if(j % 2 == 0) {
            //     randomBurstTime = rand() % abs(MAX_CPU_BURST_TIME-MIN_CPU_BURST_TIME) + MIN_CPU_BURST_TIME + 1;
            // } else {
            //     randomBurstTime = rand() % abs(MAX_IO_BURST_TIME-MIN_IO_BURST_TIME) + MIN_IO_BURST_TIME + 1;
            // }
            
            burstVector.push_back(30);
        }
        randomArrivalTime = rand() % MAX_ARRIVAL + 1;
        x[i] = PCB(i, randomPriority, 0, burstVector, randomArrivalTime);
    }
}
void print_PCBs(PCB plist[PROCESS_COUNT]) {
    for(int i = 0; i < PROCESS_COUNT; i++){
        printf("PID: %d, Priority: %d, State: %d, Burst: %d, Arrival: %d", plist[i].PID, plist[i].priority, plist[i].state, plist[i].burstTime.size(), plist[i].arrivalTime);
        printf(" T: ");
        int length = plist[i].burstTime.size();
        for(int j = 0; j < length; j++) {
            printf("%d, ", plist[i].burstTime[j]);
        }
        printf("\n");
    }

}

// FEL starts with an initial set of Arrival Events
void init_FEL(PCB plist[PROCESS_COUNT]) {
    for(int i = 0; i < PROCESS_COUNT; i++) {
        FEL.push_back(Event(1, plist[i].arrivalTime, plist[i].PID));
    }
    //printf("FEL initialized!\n");
}

Event find_imminentEvent()
{
    Event imminentEvent;
    int lowestTime = MAX_SIM_TIME; // MUST be bigger than any of the times in FEL
    int idOfLowestEventTime = -1;

    //loop through fel
    for(int i = 0; i < FEL.size(); i++) {
        if(FEL[i].eventTime < lowestTime) {
            lowestTime = FEL[i].eventTime; // find lowest time
            idOfLowestEventTime = i;
        }
    }
    if(FEL.size() > 0) {
        imminentEvent = FEL[idOfLowestEventTime];
        FEL.erase(FEL.begin()+idOfLowestEventTime);
        //printf("Lowest Time: %d\n", lowestTime);
        currentTime = lowestTime;
    }
    return imminentEvent;
}

int findHighestPriorityJob() {
    int qlistSize = 0, overallPriority, tempPriority, qID = -1;
    PCB* job;
    // find size of qlist
    for(int i = 0; i < QUEUE_COUNT; i++) {
        qlistSize += qlist[i].size();
    }
    if(qlistSize == 0) {
        return -1;
    }

    for(int i = 0; i < QUEUE_COUNT; i++) {
        if(qID == -1) {
            if(qlist[i].size() > 0){
                qID = i;
                job = qlist[i].front(); // get first job if job = NULL
                overallPriority = (currentTime - job->arrivalTimes.back()) * (110 - job->priority*10)/100;
            }
        }
        if(qlist[i].size() > 0){ // if there is something in the queue
            // calculate its overall priority
            tempPriority = (currentTime - qlist[i].front()->arrivalTimes.back()) * (110 - qlist[i].front()->priority*10)/100;

            if(tempPriority > overallPriority) {
                qID = i;
                overallPriority = tempPriority;
                job = qlist[i].front();
            }
            // overallPriority = waitTime * (110 - priority*10)/100

        }
        
    }
    //delete job;
    job = nullptr;

    // if(qID != 0) {
    //     printf("SECOND QUEUE LETS JOB INTO CPU...\n");
    // }
    return qID;
}
int getFCFSServiceTime(PCB* job) {
    int serviceTime = job->burstTime[0] + currentTime; // FCFS
    job->burstTime.erase(job->burstTime.begin()); // erase first burst
    return serviceTime;
}
int getRRServiceTime(PCB* job) {
    int serviceTime;
    if(job->burstTime[0] <= quantum) {
        serviceTime = job->burstTime[0] + currentTime;
        job->burstTime.erase(job->burstTime.begin()); // erase first burst
    } else {
        serviceTime = quantum + currentTime; // first cpu burst reduced by quantum
        job->burstTime[0] -= quantum;
    }
    return serviceTime;
}
void onCPUEntry(PCB* job, int id) {
    int serviceTime;
    switch (queueTypeList[id])
        {
        case 1:
            // FCFS
            serviceTime = getFCFSServiceTime(job);
            break;
        case 2: 
            // RR
            // generate a service time based on quantum
            serviceTime = getRRServiceTime(job);
            break;
        case 3:
            // SPN
            // generates service time the same as FCFS
            serviceTime = getFCFSServiceTime(job);
            break;
        case 4:
            // HRRN
            // generates a service time the same as FCFS
            serviceTime = getFCFSServiceTime(job);
            break;
        default:
            break;
        }
        
        serviceTime += switchTime;
        accumulatedSwitchTime += switchTime;
        job->cpuAccessTimes.push_back(serviceTime); // record departure time
        job->waitTimes.push_back(currentTime - (job->arrivalTimes.back()));
        // schedule departure event
        FEL.push_back(Event(2, serviceTime, job->PID));
}

void arrivalEvent(Event event) {
    //printf("Arrival \tPID: %d \tTime: %d \tPriority: %d\n", event.pid, currentTime, plist[event.pid].priority);
    PCB* job = &plist[event.pid];

    job->arrivalTimes.push_back(currentTime); // record arrival time
    if(SERVER_POOL < CPU_COUNT) { // if there is an idle server
        // set server to busy
        //SERVER_POOL = 1;
        SERVER_POOL += 1; // use a CPU
        cpuTimeUsed -= currentTime; // subtract unused time

        onCPUEntry(job, 0); // 0 because if there are no jobs in any queue, use the first queue's scheduling algo

    } else { // if servers are busy
        // add job to MLFQ, based on priority
        if(job->priority <= QUEUE_COUNT) {
            if(queueTypeList[job->priority-1] == 3 && qlist[job->priority-1].size() != 0) { // Code for SPN
                // insert job into correct position
                queue<PCB*> newQ;
                bool jobPushed = false;
                for(int i = 0; i < qlist[job->priority-1].size() + 1; i++) {
                    // pop out of first queue
                    if(!jobPushed && qlist[job->priority-1].size() != 0 && qlist[job->priority-1].front()->burstTime[0] > job->burstTime[0]) { // if job hasnt already been pushed into new Q
                        newQ.push(job);
                        jobPushed = true;
                    } else {
                        newQ.push(qlist[job->priority-1].front());
                        qlist[job->priority-1].pop();
                    }
                    // push into new queue if it has a smaller burst time than the incoming job
                }
                qlist[job->priority-1] = newQ; // replace queue
            } else if(queueTypeList[job->priority-1] == 4 && qlist[job->priority-1].size() != 0) {
                queue<PCB*> newQ;
                bool jobPushed = false;
                int w2 = job->getWaitingTime();
                int b2 = job->burstTime[0];
                double rr2 = ((double)w2 + (double)b2)/(double)b2;
                //cout << "Response ratio 2: " << rr2 << endl;
                for(int i = 0; i < qlist[job->priority-1].size() + 1; i++) {
                    int w1 = qlist[job->priority-1].front()->getWaitingTime();
                    
                    int b1 = qlist[job->priority-1].front()->burstTime[0];
                    
                    double rr1 = ((double)w1 + (double)b1)/(double)b1;
                    //cout << "Response ratio 1: " << rr1 << endl;
                    // pop out of first queue
                    if(!jobPushed && rr2 > rr1) { // if job hasnt already been pushed into new Q
                        newQ.push(job);
                        jobPushed = true;
                        //cout << "New job pushed" << endl;
                    } else {
                        newQ.push(qlist[job->priority-1].front());
                        qlist[job->priority-1].pop();
                        //cout << "Old job pushed" << endl;
                    }
                    // push into new queue if it has a smaller burst time than the incoming job
                }
                qlist[job->priority-1] = newQ; // replace queue
            }
            else {
                qlist[job->priority-1].push(job);
            }
        } else {
            qlist[QUEUE_COUNT-1].push(job);
        }
    }
    //delete job;
    job = nullptr;
}
void onCPUExit(Event e) {
    if(plist[e.pid].burstTime.size() > 0) { // if there are bursts still yet to be completed
        int ioTime = 0;
        if(plist[e.pid].burstTime.size() % 2 == 0) {
            // first burst is IO time, schedule arrival at currentTime + IO
            ioTime = plist[e.pid].burstTime[0];
        }
        //printf("IO \tPID: %d Time: %d\n", e.pid, ioTime);
        FEL.push_back(Event(1, currentTime+ioTime, e.pid));
        
        plist[e.pid].state += 1; // increase # of times process has been rescheduled
        if(plist[e.pid].state % QUEUE_ITTR_LIMIT == 0 && plist[e.pid].state != 0) { // if job has gone through queue 3 times
            // lower priority if it isnt already at the lowest priority level
            if(plist[e.pid].priority != PRIORITY_LEVELS) {
                plist[e.pid].priority++;
            }
        }
        if(plist[e.pid].burstTime.size() % 2 == 0) {
            plist[e.pid].burstTime.erase(plist[e.pid].burstTime.begin()); // erase IO time
        }
        
    } else { // record times
        plist[e.pid].turnaroundTime = currentTime - plist[e.pid].arrivalTime;
        plist[e.pid].responseTime = plist[e.pid].waitTimes[0];
        plist[e.pid].waitingTime = 0;
        for(int z = 0; z < plist[e.pid].waitTimes.size(); z++) {
            plist[e.pid].waitingTime += plist[e.pid].waitTimes[z];
        }   
    }
}

void departureEvent(Event event) {
    //printf("Departure \tPID: %d \tTime: %d \tPriority: %d\n", event.pid, currentTime, plist[event.pid].priority);
    PCB* nextJob;
    int jobsWaiting = 0, qid = 0;

    // find how many jobs are waiting
    for(int i = 0; i < QUEUE_COUNT; i++) {
        jobsWaiting += qlist[i].size();
    }

    if(jobsWaiting > 0) { // if there are jobs waiting in the queue(s)
        qid = findHighestPriorityJob();
        // remove job from its queue
        nextJob = qlist[qid].front();
        qlist[qid].pop();
        onCPUEntry(nextJob, qid); // second param is the queue the next job is in
    } else { // if there are no jobs waiting in the queue(s)
        // set server to idle
        SERVER_POOL -= 1;
        cpuTimeUsed += currentTime; // add used time
    }
    nextJob = nullptr;
    onCPUExit(event);
}

void printWaitingTimes() {
    int x = 0, length = -1;
    for(int i = 0; i < PROCESS_COUNT; i++) {
        printf("PID: %d Waiting Times: ", plist[i].PID);
        length = plist[i].arrivalTimes.size();
        //printf("%d, ", plist[i].arrivalTimes.at(0));
        for(int j = 0; j < length; j++) {
            printf("%d, ", plist[i].waitTimes[j]);
        }
        printf("\n");
    }
}

void printData() {
    for(int i = 0; i < PROCESS_COUNT; i++) {
        printf("PID: %d Turnaround: %d Wait: %d Response: %d\n", plist[i].PID, plist[i].turnaroundTime, plist[i].waitingTime, plist[i].responseTime);
    }
}

void printAvgs() {
    double turnaroundAvg = 0, waitAvg = 0, responseAvg = 0, timeFrame = 1000;
    double cpuUsage = (double)cpuTimeUsed / (double)(currentTime * CPU_COUNT) * 100;
    double throughput = (double)PROCESS_COUNT / (double)currentTime * timeFrame;
    for(int i = 0; i < PROCESS_COUNT; i++) {
        turnaroundAvg += plist[i].turnaroundTime;
        waitAvg += plist[i].waitingTime;
        responseAvg += plist[i].responseTime;
    }
    turnaroundAvg /= PROCESS_COUNT;
    waitAvg /= PROCESS_COUNT;
    responseAvg /= PROCESS_COUNT;

    printf("\t\tRESULTS\n");
    printf("Number of CPUs: \t%d\n", CPU_COUNT);
    printf("Number of processes: \t%d\n", PROCESS_COUNT);
    printf("Scheduling Algorithm: \t");
    for(int i = 0; i < QUEUE_COUNT; i++) {
        if(i != 0) {
            printf(", ");
        }
        switch(queueTypeList[i]) {
            case 1:
                printf("FCFS");
                break;
            case 2:
                printf("RR");
                break;
            case 3:
                printf("SPN");
                break;
            case 4:
                printf("HRRN");
                break;
            default:
                break;
        }
    }
    printf("\nThroughput: \t\t");
    cout << std::fixed;
    cout << std::setprecision(2);
    cout << throughput << " processes per " << timeFrame << " units of time" << endl;
    printf("Turnaround Average: \t");
    cout << turnaroundAvg << endl;
    printf("Wait Average: \t\t");
    cout << waitAvg << endl;
    printf("Response Average: \t");
    cout << responseAvg << endl;
    cout << "CPU Usage: \t\t" << cpuUsage << "%" << endl;
    printf("Time of completion: \t%d\n", currentTime);
    printf("Accum. Switch Time: \t");
    cout << accumulatedSwitchTime << endl;
}

int main(){
    srand(time(0));
    Event imminentEvent;

    create_PCBs(plist);
    //print_PCBs(plist);

    init_FEL(plist); // add arrival events from process list

    // main event loop
    while(FEL.size() > 0){
        imminentEvent = find_imminentEvent(); // find imminent event
        
        //FEL.remove(imminentEventId);
        switch (imminentEvent.type)
        {
            case 1:
                arrivalEvent(imminentEvent);
                break;
            case 2:
                departureEvent(imminentEvent);
                break;
            default:
                break;
        }
    }
    //print_PCBs(plist);
    //printWaitingTimes();

    //printData();
    printAvgs();
    return 0;
}
