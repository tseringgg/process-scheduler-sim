#include <vector>
#ifndef PCB_H
#define PCB_H


class PCB {
    public:
        int PID;
        int priority;
        int state; // # of times ran through queue
        std::vector<int> burstTime; // time needed to complete execution
        std::vector<int> arrivalTimes; // time that process arrives into queue
        std::vector<int> waitTimes;
        std::vector<int> cpuAccessTimes; // time that process gains cpu access
        int turnaroundTime;
        int waitingTime;
        int responseTime;
        int arrivalTime;
        int getWaitingTime(){
            int sum = 0;
            for(int i = 0; i < waitTimes.size(); i++) {
                sum += waitTimes[i];
            }
            return sum;
        }
        PCB(int _PID, int _priority, int _state, std::vector<int> _burstTime, int _arrivalTime){
            PID = _PID;
            priority = _priority;
            state = _state;
            burstTime = _burstTime;
            arrivalTime = _arrivalTime;
            turnaroundTime = 0;
            waitingTime = 0;
            responseTime = 0;
        }
        PCB(){

        }
};

#endif