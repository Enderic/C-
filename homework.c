#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

struct Process{
    int processID;
    int arrivalTime;
    int burstTime;
    int completionTime;
    int turnAroundTime;
    int waitTime;
    int responseTime;
    int initialTime;
    bool firstTime;
    int priority;
};

// Function to create Processes
struct Process createProcess(int processID, int arrivalTime, int burstTime, int priority) {
    struct Process p;
    p.processID = processID;
    p.arrivalTime = arrivalTime;
    p.burstTime = burstTime;
    p.completionTime = 0;
    p.waitTime = 0;
    p.turnAroundTime = 0;
    p.responseTime = 0;
    p.initialTime = 0;
    p.firstTime = true;
    p.priority = priority;
    return p;
}

// All functions for scheduling algorithms (Maybe move to another file for cleanliness?)
// Each function below the main ones are helper functions specific to that scheduling algorithm
void FirstComeFirstServe(struct Process table[], int size);
struct Process FCFSFindProcessToExecute(struct Process processes[], int size, int time);

void ShortestTimeRemainingFirst(struct Process table[], int size);
struct Process STRFFindProcessToExecute(struct Process processes[], int size, int time);

void RoundRobin(struct Process table[], int size);
void RRFillInReadyQueue(struct Process processes[], struct Process readyQueue[], int size, int time);
struct Process RRFindProcessToExecute(struct Process readyQueue[]);
void RRUpdateReadyQueue(struct Process readyQueue[], struct Process processToMoveBack, int size);
int FindProcessIndexByID(struct Process tableToSearch[], int size, int id);
void BubbleSortByArrivalTime(struct Process arr[], int size);
void RRUpdateOutputProcessValue(struct Process outputTable[], struct Process readyQueue[], int time, struct Process processToExecute, int* averageWaitingTime, int* averageTurnaroundTime, int* averageResponseTime);

void PreemptivePriorityScheduling(struct Process table[], int size);
struct Process PPSFindProcessToExecute(struct Process processes[], int size, int time);

// Overall Helper Functions
void CopyToTable(struct Process table[], struct Process tableToCopy[], int size);
bool IsProcessEmpty(struct Process processes[], int size);
void UpdateOutputProcessValue(struct Process outputTable[], struct Process processTable[], int time, struct Process processToExecute, int* averageWaitingTime, int* averageTurnaroundTime, int* averageResponseTime);
void OutputTable(struct Process outputTable[], int size, int averageWaitingTime, int averageTurnaroundTime, int averageResponseTime, int totalTime);

int main(int argc, char* argv[]) {

    // Gets how many processes the user wants to create
    int amountOfProcess = 0;

    printf("Enter amount of processes: ");
    scanf("%d", &amountOfProcess);

    if((amountOfProcess >= 50) || (amountOfProcess <= 0))
    {
        printf("Not a good amount of processes");
        return 1;
    }

    // A table with all the processes
    struct Process processTable[amountOfProcess];

    // Used for more accurate randomness with rand()
    srand(time(NULL));

    // Creates all the random processes w/ random values
    for(int i = 0; i < amountOfProcess; i++)
    {
        int priority = rand() % 50;
        int burstTime = rand() % 20;
        if(burstTime == 0)
        {
            burstTime = 1;
        }
        int arrivalTime = rand() % 25;
        struct Process newProcess = createProcess(i + 1, arrivalTime, burstTime, priority);
        processTable[i] = newProcess;
    }

    for(int i = 0; i < amountOfProcess; i++)
    {
        printf("Process %d -> Arrival Time: %d, Burst Time: %d, Priority: %d\n", processTable[i].processID, processTable[i].arrivalTime, processTable[i].burstTime, processTable[i].priority);
    }
    printf("\nFCFS\n");
    FirstComeFirstServe(processTable, amountOfProcess);
    printf("\nSTRF\n");
    ShortestTimeRemainingFirst(processTable, amountOfProcess);
    printf("\nRound Robin\n");
    RoundRobin(processTable, amountOfProcess);
    printf("\nPPS\n");
    PreemptivePriorityScheduling(processTable, amountOfProcess);
}

void FirstComeFirstServe(struct Process table[], int size) {
    int time = 0;

    int averageWaitingTime = 0;
    int averageTurnaroundTime = 0;
    int averageResponseTime = 0;
    int throughput = 0;

    bool isDone = false;

    struct Process processes[size];
    CopyToTable(processes, table, size);

    struct Process outputTable[size];
    CopyToTable(outputTable, table, size);

    struct Process processToExecute = createProcess(-1, -1, -1, -1);
    printf("-------------------------------------------------------------------------------------------\n");
    while(!isDone)
    {
        processToExecute = FCFSFindProcessToExecute(processes, size, time);
        if(processToExecute.processID == -1)
        {
            ++time;
            printf("|");
            continue;
        }

        if(processToExecute.firstTime)
        {
            processes[processToExecute.processID - 1].firstTime = false;
            processes[processToExecute.processID - 1].initialTime = time;
        }

        time += processToExecute.burstTime;
        printf("|P_%d T -> %d|", processToExecute.processID, time);

        UpdateOutputProcessValue(outputTable, processes, time, processToExecute, &averageWaitingTime, &averageTurnaroundTime, &averageResponseTime);

        isDone = IsProcessEmpty(processes, size);
    }
    printf("\n------------------------------------------------------------------------------------------------\n");
    OutputTable(outputTable, size, averageWaitingTime, averageTurnaroundTime, averageResponseTime, time);
}

struct Process FCFSFindProcessToExecute(struct Process processes[], int size, int time) {
    struct Process toExecute = createProcess(-1, 100, -1, -1);
    int index = -1;
    for(int i = 0; i < size; i++)
    {
        if(processes[i].processID == -1) // Valid Process
            continue;

        if(processes[i].arrivalTime > time) // Is in RQ
            continue;

        if(processes[i].burstTime <= 0) // Not already done process
            continue;

        if (processes[i].arrivalTime < toExecute.arrivalTime)
        {
            toExecute = processes[i];
            index = i;
            continue;
        }
        else
        {
            if (processes[i].processID < toExecute.processID)
            {
                toExecute = processes[i];
                index = i;
                continue;
            }
        }
    }
    if (index != -1)
    {
        processes[index] = createProcess(-1, -1, -1, -1);
    }
    return toExecute;
}

void ShortestTimeRemainingFirst(struct Process table[], int size) {
    int time = 0;

    int averageWaitingTime = 0;
    int averageTurnaroundTime = 0;
    int averageResponseTime = 0;

    bool isDone = false;

    struct Process processes[size];
    CopyToTable(processes, table, size);

    struct Process outputTable[size];
    CopyToTable(outputTable, table, size);

    int processOriginalBurstTime[size];
    for(int i = 0; i < size; i++)
    {
        processOriginalBurstTime[i] = processes[i].burstTime;
    }

    struct Process processToExecute = createProcess(-1, -1, -1, -1);
    printf("-------------------------------------------------------------------------------------------\n");
    while(!isDone)
    {
        processToExecute = STRFFindProcessToExecute(processes, size, time);
        if(processToExecute.processID == -1)
        {
            ++time;
            printf("|");
            continue;
        }

        if(processToExecute.firstTime)
        {
            processes[processToExecute.processID - 1].firstTime = false;
            processes[processToExecute.processID - 1].initialTime = time;
        }
        ++time;
        printf("|P_%d Time -> %d|", processToExecute.processID, time);
        processes[processToExecute.processID - 1].burstTime -= 1;
        if(processes[processToExecute.processID - 1].burstTime == 0)
        {
            processToExecute.burstTime = processOriginalBurstTime[processToExecute.processID - 1];
            UpdateOutputProcessValue(outputTable, processes, time, processToExecute, &averageWaitingTime, &averageTurnaroundTime, &averageResponseTime);
        }
        isDone = IsProcessEmpty(processes, size);
    }
    printf("\n------------------------------------------------------------------------------------------------\n");
    OutputTable(outputTable, size, averageWaitingTime, averageTurnaroundTime, averageResponseTime, time);
}

struct Process STRFFindProcessToExecute(struct Process processes[], int size, int time) {
    struct Process toExecute = createProcess(-1, -1, 100, -1);
    for(int i = 0; i < size; i++)
    {
        if(processes[i].processID == -1) // Valid Process
            continue;

        if(processes[i].arrivalTime > time) // Is in RQ
            continue;

        if(processes[i].burstTime <= 0) // Not already done process
            continue;

        if(processes[i].burstTime < toExecute.burstTime) // Has less BT
        {
            toExecute = processes[i];
            continue;
        }

        if(processes[i].burstTime == toExecute.burstTime) // Equal BT
        {
            if(processes[i].arrivalTime < toExecute.arrivalTime) // Checks if AT is less
            {
                toExecute = processes[i];
                continue;
            }
            else if(processes[i].arrivalTime == toExecute.arrivalTime) // Checks if AT is equal
            {
                if(processes[i].processID < toExecute.processID) // Checks if ID is less
                {
                    toExecute = processes[i];
                    continue;
                }
            }
        }
    }

    return toExecute;
}

void RoundRobin(struct Process table[], int size) {
    int time = 0;

    int averageWaitingTime = 0;
    int averageTurnaroundTime = 0;
    int averageResponseTime = 0;

    bool isDone = false;

    struct Process processes[size];
    CopyToTable(processes, table, size);

    struct Process outputTable[size];
    CopyToTable(outputTable, table, size);

    int processOriginalBurstTime[size];
    for(int i = 0; i < size; i++)
    {
        processOriginalBurstTime[i] = processes[i].burstTime;
    }

    struct Process readyQueue[size + 1];
    for(int i = 0; i < size + 1; i++)
    {
        readyQueue[i] = createProcess(-1, -1, -1, -1);
    }

    int timeQuantum = rand() % 7 + 1;
    printf("TQ = %d\n", timeQuantum);

    struct Process processToExecute = createProcess(-1, -1, -1, -1);
    printf("-------------------------------------------------------------------------------------------\n");
    RRFillInReadyQueue(processes, readyQueue, size, time);
    while(!isDone)
    {
        processToExecute = RRFindProcessToExecute(readyQueue);
        if(processToExecute.processID == -1)
        {
            ++time;
            RRFillInReadyQueue(processes, readyQueue, size, time);
            printf("|");
            continue;
        }
        if(processToExecute.firstTime == true)
        {
            int index = FindProcessIndexByID(readyQueue, size, processToExecute.processID);
            readyQueue[index].firstTime = false;
            readyQueue[index].initialTime = time;
        }

        if(processToExecute.burstTime > timeQuantum)
        {
            time += timeQuantum;
            processToExecute.burstTime -= timeQuantum;
            printf("|P_%d Time -> %d|", processToExecute.processID, time);
            RRFillInReadyQueue(processes, readyQueue, size, time);
            RRUpdateReadyQueue(readyQueue, processToExecute, size);
        }
        else
        {
            time += processToExecute.burstTime;
            printf("|P_%d Time -> %d|", processToExecute.processID, time);
            int index = processToExecute.processID - 1;
            processToExecute.burstTime = processOriginalBurstTime[index];
            RRUpdateOutputProcessValue(outputTable, readyQueue, time, processToExecute, &averageWaitingTime, &averageTurnaroundTime, &averageResponseTime);
            RRFillInReadyQueue(processes, readyQueue, size, time);
            RRUpdateReadyQueue(readyQueue, createProcess(-1, -1, -1, -1), size);
        }
        isDone = IsProcessEmpty(processes, size) && IsProcessEmpty(readyQueue, size);
    }
    printf("\n------------------------------------------------------------------------------------------------\n");
    OutputTable(outputTable, size, averageWaitingTime, averageTurnaroundTime, averageResponseTime, time);
}

void RRFillInReadyQueue(struct Process processes[], struct Process readyQueue[], int size, int time) {
    struct Process processesToStore[size];
    // Initializes processesToStore table
    for(int i = 0; i < size; i++)
    {
        processesToStore[i] = createProcess(-1, -1, -1, -1);
    }

    for(int i = 0, j = 0; i < size; i++)
    {
        if(processes[i].processID == -1) // Valid Process
            continue;

        if(processes[i].arrivalTime > time) // Is in RQ
            continue;

        if(processes[i].burstTime <= 0) // Not already done process
            continue;

        if(processes[i].arrivalTime <= time)
        {
            processesToStore[j] = processes[i];
            processes[i] = createProcess(-1, -1, -1, -1);
            ++j;
        }
    }

    BubbleSortByArrivalTime(processesToStore, size);
    for(int i = 0, j = 0; i < size; NULL)
    {
        if(processesToStore[i].processID == -1)
            break;

        if(readyQueue[j].processID != -1)
        {
            ++j;
            continue;
        }
        readyQueue[j] = processesToStore[i];
        ++j;
        ++i;
    }
}

struct Process RRFindProcessToExecute(struct Process readyQueue[]) {
    return readyQueue[0];
}

void RRUpdateReadyQueue(struct Process readyQueue[], struct Process processToMoveBack, int size) {
    bool isDone = false;
    int index = 0;

    while(!isDone) {
        if(readyQueue[index + 1].processID == -1)
        {
            readyQueue[index] = processToMoveBack;
            isDone = true;
            continue;
        }
        if (index == size + 1)
            break;

        readyQueue[index] = readyQueue[index + 1];
        index++;
    }
}

int FindProcessIndexByID(struct Process tableToSearch[], int size, int id) {
    for(int i = 0; i < size; i++)
    {
        if(tableToSearch[i].processID == id)
        {
            return i;
        }
    }
    return -1;
}

void BubbleSortByArrivalTime(struct Process arr[], int size) {
    for (int i = 0; i < size - 1; i++) {
        for (int j = 0; j < size - i - 1; j++) {
            if (arr[j].processID == -1 || arr[j + 1].processID == -1)
                continue;

            if (arr[j].arrivalTime > arr[j + 1].arrivalTime) {
                struct Process temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

void RRUpdateOutputProcessValue(struct Process outputTable[], struct Process readyQueue[], int time, struct Process processToExecute, int* averageWaitingTime, int* averageTurnaroundTime, int* averageResponseTime) {
    int CT = time;
    int TAT = CT - processToExecute.arrivalTime;
    int WT = TAT - processToExecute.burstTime;
    int RT = readyQueue[0].initialTime - processToExecute.arrivalTime;
    struct Process newInfo = {processToExecute.processID,
                              processToExecute.arrivalTime,
                              processToExecute.burstTime,
                              CT,
                              TAT,
                              WT,
                              RT,
                              processToExecute.initialTime,
                              false,
                              processToExecute.priority};

    outputTable[newInfo.processID - 1] = newInfo;

    *averageWaitingTime += WT;
    *averageTurnaroundTime += TAT;
    *averageResponseTime += RT;
}

void PreemptivePriorityScheduling(struct Process table[], int size) {
    int time = 0;

    int averageWaitingTime = 0;
    int averageTurnaroundTime = 0;
    int averageResponseTime = 0;

    bool isDone = false;

    struct Process processes[size];
    CopyToTable(processes, table, size);

    struct Process outputTable[size];
    CopyToTable(outputTable, table, size);

    int processOriginalBurstTime[size];
    for(int i = 0; i < size; i++)
    {
        processOriginalBurstTime[i] = processes[i].burstTime;
    }

    struct Process processToExecute = createProcess(-1, -1, -1, -1);
    printf("-------------------------------------------------------------------------------------------\n");
    while(!isDone)
    {
        processToExecute = PPSFindProcessToExecute(processes, size, time);
        if(processToExecute.processID <= -1 || processToExecute.processID > 50)
        {
            ++time;
            printf("|");
            continue;
        }

        if(processToExecute.firstTime)
        {
            processes[processToExecute.processID - 1].firstTime = false;
            processes[processToExecute.processID - 1].initialTime = time;
        }
        ++time;
        printf("|P_%d Time -> %d|", processToExecute.processID, time);
        processes[processToExecute.processID - 1].burstTime -= 1;
        if(processes[processToExecute.processID - 1].burstTime == 0)
        {
            processToExecute.burstTime = processOriginalBurstTime[processToExecute.processID - 1];
            UpdateOutputProcessValue(outputTable, processes, time, processToExecute, &averageWaitingTime, &averageTurnaroundTime, &averageResponseTime);
        }
        isDone = IsProcessEmpty(processes, size);
    }
    printf("\n------------------------------------------------------------------------------------------------\n");
    OutputTable(outputTable, size, averageWaitingTime, averageTurnaroundTime, averageResponseTime, time);
}

struct Process PPSFindProcessToExecute(struct Process processes[], int size, int time) {
    struct Process toExecute = createProcess(-1, -1, 100, -1);
    for(int i = 0; i < size; i++)
    {
        if(processes[i].processID == -1) // Valid Process
            continue;

        if(processes[i].arrivalTime > time) // Is in RQ
            continue;

        if(processes[i].burstTime <= 0) // Not already finished process
            continue;

        if(processes[i].priority > toExecute.priority)
        {
            toExecute = processes[i];
            continue;
        }
        else if(processes[i].priority == toExecute.priority)
        {
            if(processes[i].arrivalTime < toExecute.arrivalTime)
            {
                toExecute = processes[i];
                continue;
            }
            else if(processes[i].arrivalTime == toExecute.arrivalTime)
            {
                if(processes[i].burstTime > toExecute.burstTime)
                {
                    toExecute = processes[i];
                    continue;
                }
                else if(processes[i].burstTime == toExecute.burstTime)
                {
                    if(processes[i].processID < toExecute.processID)
                    {
                        toExecute = processes[i];
                        continue;
                    }
                }
            }
        }
    }

    return toExecute;
}

bool IsProcessEmpty(struct Process processes[], int size) {
    for(int i = 0; i < size; i++)
    {
        if(processes[i].processID != -1)
        {
            return false;
        }
    }
    return true;
}

void CopyToTable(struct Process table[], struct Process tableToCopy[], int size) {
    for(int i = 0; i < size; i++)
    {
        table[i] = tableToCopy[i];
    }
}

void UpdateOutputProcessValue(struct Process outputTable[], struct Process processTable[], int time, struct Process processToExecute, int* averageWaitingTime, int* averageTurnaroundTime, int* averageResponseTime) {
    int CT = time;
    int TAT = CT - processToExecute.arrivalTime;
    int WT = TAT - processToExecute.burstTime;
    int RT = processTable[processToExecute.processID - 1].initialTime - processToExecute.arrivalTime;
    struct Process newInfo = {processToExecute.processID,
                              processToExecute.arrivalTime,
                              processToExecute.burstTime,
                              CT,
                              TAT,
                              WT,
                              RT,
                              processToExecute.initialTime,
                              false,
                              processToExecute.priority};

    outputTable[newInfo.processID - 1] = newInfo;
    processTable[newInfo.processID - 1] = createProcess(-1, -1, -1, -1);

    *averageWaitingTime += WT;
    *averageTurnaroundTime += TAT;
    *averageResponseTime += RT;
}

void OutputTable(struct Process outputTable[], int size, int averageWaitingTime, int averageTurnaroundTime, int averageResponseTime, int totalTime) {
    for(int i = 0; i < size; i++)
    {
        printf("Process %d -> AT: %d, BT: %d, CT: %d, TAT: %d, WT: %d, RT: %d, Priority: %d\n", outputTable[i].processID,
               outputTable[i].arrivalTime,
               outputTable[i].burstTime,
               outputTable[i].completionTime,
               outputTable[i].turnAroundTime,
               outputTable[i].responseTime,
               outputTable[i].waitTime,
               outputTable[i].priority);
    }
    printf("\nAverage WT: %.4f, Average TAT: %.4f, Average RT: %.4f, TP: %.4f",
           (float)averageWaitingTime / size,
           (float)averageTurnaroundTime / size,
           (float)averageResponseTime / size,
           (float)totalTime / size);
}