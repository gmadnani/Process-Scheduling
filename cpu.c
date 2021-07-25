#include "cpu.h"

// Struct definitions
typedef struct {
    unsigned int processors;
    char * processListFile;
    unsigned int useOwnScheduler;
    void * hProcs; // list of processes
    void * hProcessors; // list of processors
    unsigned int unfinished; // number of pending processes 
    unsigned int finished;
} CPUINFO;

// Helper functions declaration

// Loads processes from file
static void loadProcesses(CPUINFO * cpuInfo);

#define INFO(h) if (!h) { return 0; } CPUINFO * info = (CPUINFO*)h;
#define INFON(h) if (!h) { return; } CPUINFO * info = (CPUINFO*)h;

// Reads the executable's arguments and extracts the options.
// Returns the pointer of the CPUINFO struct if the options are valid and read successfully.
// Returns NULL otherwise.
void * cpuInit(int argc, char** argv) {
     CPUINFO* info = (CPUINFO*)calloc(1, sizeof(CPUINFO));

    if (info == NULL) {
        fprintf(stderr, "Failed to allocate memory for cpu info.\n");
        return NULL;
    }

    int options = 0x0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-f") == 0) {
            if (i + 1 == argc) {
                fprintf(stderr, "-f option expects an input file name.\n");
                cpuDelete(info);
                return NULL;
            }
            int len = strlen(argv[i+1]);
            info->processListFile = (char *)calloc(len + 1, sizeof(char));
            strcpy(info->processListFile, argv[i+1]); // copy input to string to info
            if (info->processListFile == NULL) {
                fprintf(stderr, "Failed to allocate memory for process list file.\n");
                cpuDelete(info);
                return NULL;
            }

            options |= 0x1; // flag -f is completed
            i++; // skip
        } else if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 == argc) {
                fprintf(stderr, "-p option expects an input on the number of processors.\n");
                cpuDelete(info);
                return NULL;
            }
            info->processors = atoi(argv[i+1]);
            i++; // skip
            options |= 0x2; // flag -p is completed
        } else if (strcmp(argv[i], "-c") == 0) {
            info->useOwnScheduler = 1;
        } else {
            // skip undefined commands
        }
    }

    // check if all options are read from the argument.
    if (!(options & 0x3)) {
        cpuDelete(info);
        fprintf(stderr, "Process scheduler lacks one or more options from the arguments.\n");
        return NULL;
    }

    if (info->processors == 0) {
        cpuDelete(info);
        fprintf(stderr, "No processor to be made.\n");
        return NULL;
    }

    // load processes into procs list
    loadProcesses(info);
    if (info->hProcs == NULL) {
        cpuDelete(info);
        fprintf(stderr, "Failed to create list of processes.\n");
        return NULL;
    }

    // create list of processes as queue for each core
    info->hProcessors = listCreate();
    if (info->hProcessors == NULL) {
        cpuDelete(info);
        fprintf(stderr, "Failed to create cores.\n");
        return NULL;
    }

    // create the processors
    void * processor = NULL;
    for (unsigned int i = 0; i < info->processors; i++) {
        
        processor = processorCreate(i);
        if (!processor) {
            cpuDelete(info);
            fprintf(stderr, "Failed to create processor.\n");
            return NULL;
        }

        listInsert(info->hProcessors, i, processor);
    }


    return info; // done initialization
}

// delete the cpu
void cpuDelete(void * cpuHandle) {
    INFON(cpuHandle)

    // Delete process list file
    if (info->processListFile) {
        free(info->processListFile);
    }
    
    // Delete processes
    size_t count = listCount(info->hProcs);
    for (size_t i = 0; i < count; i++) {
        procDelete(listGet(info->hProcs, i));
    }
    listDelete(info->hProcs);

    // Delete processors
    count = listCount(info->hProcessors);
    for (size_t i = 0; i < count; i++) {
        processorDelete(listGet(info->hProcessors, i));
    }
    listDelete(info->hProcessors);

    free(info);
}

// run a single frame in the cpu, takes the cpu handle and current time as parameters
// returns 0 if processes are completed/error 
// returns 1 otherwise
int cpuRun(void * cpuHandle, unsigned int time) {
    INFO(cpuHandle)

    if (info->useOwnScheduler) { 
        info->unfinished += cpuOwnSchedule(info, time);
    } else {
        info->unfinished += cpuSchedule(info, time);
    }

    void * finished = listCreate(); 

    if (!finished) { // failed to create list of finished processes
        return 0;
    }

    void * running = listCreate();

    if (!running) {
        listDelete(running);
        return 0;
    }

    for (unsigned int i = 0; i < info->processors; i++) {
        listInsert(running, i, NULL); // populate NULL pointers in the running
    }

    size_t count = listCount(info->hProcessors);

    // Checked finished processes
    for (size_t i = 0; i < count; i++) {
        void * hCurrentSubProc = processorCurrentSubProc(listGet(info->hProcessors, i));

        if (!hCurrentSubProc) {
            continue; 
        }

        void * hParent = subProcParent(hCurrentSubProc); 
        if (procRem(hParent) == 0) { // process is done
            // report to CPU running function
            // check if hParent is already in the list of finished procs
            if (!listSearch(finished, hParent, NULL)) {
                listPush(finished, hParent);
                info->unfinished--;
                info->finished++;
            }
        }  
    }

    // Execute each processor    
    for (size_t i = 0; i < count; i++) {
        processorRun(listGet(info->hProcessors, i), time, running);
    }

    // report finished processes
    count = listCount(finished);
    for (size_t i = 0; i < count; i++) {
        printf("%u,FINISHED,pid=%u,proc_remaining=%d\n", time, procID(listGet(finished, i)), info->unfinished);
    }

    // report running processes
    for (unsigned int i = 0; i < info->processors; i++) {
        void * hCurrentSubProc = listGet(running, i);
        if (!hCurrentSubProc) {
            continue; // skip if pointer is NULL
        }
        void * hParent = subProcParent(hCurrentSubProc); 
        if (procSubs(hParent) > 1) { // is parallel
            printf("%d,RUNNING,pid=%u.%u,remaining_time=%d,cpu=%u\n", time, procID(hParent), subProcID(hCurrentSubProc), subProcRem(hCurrentSubProc) + 1, i);
        } else {
            printf("%d,RUNNING,pid=%u,remaining_time=%d,cpu=%u\n", time, procID(hParent), subProcRem(hCurrentSubProc) + 1, i);
        }
    }
  
    listDelete(finished);
    listDelete(running);

    return info->finished != listCount(info->hProcs);    
}

/*
 * New Scheduling Algorithm:
 * The disadvantage of the Shortest Remaining Time Algorithm is that it delays longer
 * processes in favor of the shorter ones.
 */

size_t cpuOwnSchedule(void * hCPU, unsigned int time) {
    INFO(hCPU)

    // sort the new processes from shortest to longest
    void * hArrivingProcs = listCreate();
    if (!hArrivingProcs) {
        return 0;
    }

    size_t arriving = 0, insert = 0, count = 0;

    unsigned int exec = 0, exec2 = 0;

    void * hProc = NULL;
    void * hProc2 = NULL;

    unsigned int nextArrival = time;

    count = listCount(info->hProcs);

    // rank the list of arriving processes from shortest to longest execution
    for (size_t i = 0; i < count; i++) {
        hProc = listGet(info->hProcs, i);

        // get the next arrival time
        if (procArrivalTime(hProc) != time) {
            if (procArrivalTime(hProc) > time) {
                if (nextArrival == time) {
                    nextArrival = procArrivalTime(hProc);
                } else if (procArrivalTime(hProc) < nextArrival) {
                    nextArrival = procArrivalTime(hProc);
                }
            }
            continue; 
        }

        exec = subProcExecTime(listGet(procSubsHandle(hProc), 0));   

        arriving = listCount(hArrivingProcs);
        for (insert = 0; insert < arriving; insert++) {
            hProc2 = listGet(hArrivingProcs, insert);
            exec2 = subProcExecTime(listGet(procSubsHandle(hProc2), 0));  

            if (exec < exec2) {
                break;
            } else if (exec == exec2) {
                if (procID(hProc) < procID(hProc2)) {
                    break;
                }
            }
        }

        listInsert(hArrivingProcs, insert, hProc);
    }

    arriving = listCount(hArrivingProcs);

    // for each new process
    for (size_t i = 0; i < arriving; i++) {
        hProc = listGet(hArrivingProcs, i);

        void * cpuList = listCreate();

        // get a list of processors from least to greatest waiting time
        for (size_t j = 0; j < info->processors; j++) {
            void * processor = listGet(info->hProcessors, j);
            
            double f = 0;
            count = listCount(processorPending(processor));
            for (size_t k = 0; k < count; k++) {
                f += subProcWaiting(listGet(processorPending(processor), k), time);
            }
            
            // find the insertion point
            size_t cpuCount = listCount(cpuList);
            for (insert = 0; insert < cpuCount; insert++) {
                void * processor2 = listGet(cpuList, insert);
                
                double f2 = 0;
                count = listCount(processorPending(processor2));
                for (size_t k = 0; k < count; k++) {
                    f2 += subProcWaiting(listGet(processorPending(processor2), k), time);
                }

                if (f < f2) {
                    break;
                } else if (f == f2) { // in case of tie on the waiting time, break tie with the shorter remaining time
                    if (processorRemainingTime(processor) < processorRemainingTime(processor2)) {
                        break;
                    } else if (processorRemainingTime(processor) == processorRemainingTime(processor2)) {
                        if (processorID(processor) < processorID(processor2)) {
                            break;
                        }
                    }
                }
            }

            listInsert(cpuList, insert, processor);
        }

        // assign the new subprocesses to the CPUs
        count = listCount(procSubsHandle(hProc));
        for (size_t j = 0; j < count; j++) {
            void * processor = listGet(cpuList, j);
            void * pending = processorPending(processor);
            void * subProc = listGet(procSubsHandle(hProc), j);
            void * pendingSP = NULL;

            double f = 0; // waiting time is zero

            // find the insertion point of the subprocesses in the 
            size_t pendingCount = listCount(pending);
            for (insert = 0; insert < pendingCount; insert++) {
                pendingSP = listGet(pending, insert);
                // compute f of the pending sub proc
                double f2 = 0;
                
                f2 = subProcWaiting(pendingSP, time);

                if (f > f2) {
                    break;
                } else if (f == f2) { // if the waiting time is tied, select the sub process with shorter remaining time
                    if (subProcRem(subProc) < subProcRem(pendingSP)) {
                        break;
                    } else if (subProcRem(subProc) == subProcRem(pendingSP)) {
                        if (procID(hProc) < procID(subProcParent(pendingSP))) {
                            break;
                        }
                    }
                }
            }
            
            listInsert(pending, insert, subProc);
        }

        listDelete(cpuList);
    }

    return arriving;
}

// schedule the process to the cores
/*
 * Questions answered by the cpu scheduler:
 * 1. Which of the arriving processes are scheduled first?
 * 2. What cpu should each process/sub process be assigned to?
 * 3. What is the priority of the sub processes in the pending queue of each processor?
 * 
 * Answers to the questions for DEFAULT scheduler
 * 1. The arriving process with least execution time will be scheduled first.
 * 2. The cpu with least remaining time will be chosen.
 * 3. The sub process with least execution time will be prioritized by the processor.
 */

size_t cpuSchedule(void * hCPU, unsigned int time) {
    INFO(hCPU)

    // get the list of arriving sub processes and sort them from fastest to slowest execution time
    void * hArrivingProcs = listCreate();
    if (!hArrivingProcs) { // failed to create list of arriving processes
        return 0;
    }

    unsigned int fastest = 0, slowest = 0, exec = 0, exec2 = 0;
    size_t count = listCount(info->hProcs), insert = 0, arriving = 0;
    void * hProc = NULL;
    void * hProc2 = NULL;
    void * hSubProc = NULL;
    void * hSubProc2 = NULL;

    for (size_t i = 0; i < count; i++) {
        hProc = listGet(info->hProcs, i);

        if (procArrivalTime(hProc) != time) {
            continue; // skip processes not arriving
        }

        hSubProc = listGet(procSubsHandle(hProc), 0); // get first sub process
        exec = subProcExecTime(hSubProc);
        arriving = listCount(hArrivingProcs);

        if (arriving == 0) { // first arriving proc
            insert = 0;
            fastest = exec;
            slowest = exec;
        } else {
            if (exec < fastest) {
                insert = 0; // insert to first of array
                fastest = exec; 
            } else if (exec > slowest) { 
                insert = arriving; // insert to the end of the list
                slowest = exec;
            } else {
                // search for the best insertion point
                for (insert = 0; insert < arriving; insert++) {
                    hProc2 = listGet(hArrivingProcs, insert);
                    hSubProc2 = listGet(procSubsHandle(hProc2), 0); 
                    exec2 = subProcExecTime(hSubProc2);

                    if (exec == exec2) {
                        if (procID(hProc) < procID(hProc2)) {
                            break;
                        }
                    } else if (exec < exec2) {
                        break; // insert here
                    }
                }
            }
        }

        if (!listInsert(hArrivingProcs, insert, hProc)) { // failed to insert processes to the list of arriving processes
            listDelete(hArrivingProcs); // clean up memory for the list of arriving processes
            return 0;
        }
    }
    arriving = listCount(hArrivingProcs); // update count of arriving procs

    // assign the processes to the CPUs
    size_t cpusToAssign = 0, cpuListCount;
    
    for (size_t i = 0; i < arriving; i++) {
        hProc = listGet(hArrivingProcs, i);
        cpusToAssign = procSubs(hProc);

        void * cpuList = listCreate();
        if (!cpuList) { // failed to create list of processors
            listDelete(hArrivingProcs); // clean up memory
            return 0;
        }

        unsigned int cpuRemTime = 0, cpu2RemTime = 0;

        // make an ordered list of cpus from least remaining time to greatest remaining time
        for (size_t j = 0; j < info->processors; j++) {
            cpuListCount = listCount(cpuList);
            cpuRemTime = processorRemainingTime(listGet(info->hProcessors, j));                    

            for (insert = 0; insert < cpuListCount; insert++) { // search for insertion point
                cpu2RemTime = processorRemainingTime(listGet(cpuList, insert));

                if (cpuRemTime < cpu2RemTime) { // insert here
                    break;
                } else if (cpuRemTime == cpu2RemTime) {
                    if (processorID(listGet(info->hProcessors, j)) < processorID(listGet(cpuList, insert))) {
                        break;
                    }
                }
            }

            if (!listInsert(cpuList, insert, listGet(info->hProcessors, j))) { // failed to insert
                listDelete(cpuList);
                listDelete(hArrivingProcs);
                return 0;
            }
        }

        // insert the sub processes to the processors
        // for each sub process in the ariiving process
        for (size_t j = 0; j < cpusToAssign; j++) {
            void * processor = listGet(cpuList, j);
            void * pending = processorPending(processor);
            hSubProc = processorCurrentSubProc(processor); // current sub proc of processor
            hSubProc2 = listGet(procSubsHandle(hProc), j); // sub proc to insert
            
            // check if the sub process can replace the current sub process in the processor
            if (hSubProc) {
                exec = subProcRem(hSubProc); // check remaining time of current sub process
                exec2 = subProcExecTime(hSubProc2);

                if (exec2 < exec) {
                    // put the current sub proc to list of pending processes in the first
                    listInsert(pending, 0, hSubProc); // set taret to null
                    listInsert(pending, 0, hSubProc2); // insert the new sub proc at the first
                    processorClearCurrent(processor);
                    continue; // skip 
                } else if (exec == exec2) { // test pid
                    if (procID(subProcParent(hSubProc2)) < procID(subProcParent(hSubProc2))) {
                        // put the current sub proc to list of pending processes in the first
                        listInsert(pending, 0, hSubProc); // set taret to null
                        listInsert(pending, 0, hSubProc2); // insert the new sub proc at the first
                        processorClearCurrent(processor);
                        continue; // skip 
                    } 
                }
            }

            // insert the sub proc to pending
            count = listCount(pending);         
            for (insert = 0; insert < count; insert++) {
                hSubProc = listGet(pending, insert);
                exec = subProcRem(hSubProc); // check remaining time of current sub process
                exec2 = subProcExecTime(hSubProc2);
                
                if (exec2 < exec) {
                    break;
                } else if (exec2 == exec) {
                    if (procID(subProcParent(hSubProc2)) < procID(subProcParent(hSubProc))) {
                        break;
                    }
                }
            }

            if (!listInsert(pending, insert, hSubProc2)) {
                listDelete(cpuList);
                listDelete(hArrivingProcs);
                return 0;
            }
            
        }

        listDelete(cpuList);
    }

    listDelete(hArrivingProcs);

    return arriving;
}

// compute simulation statistics
void cpuStats(void * cpuHandle, unsigned int time) {
    INFON(cpuHandle);

    double sumTAT = 0.0;
    double sumOverhead = 0.0;
    double maxOverhead = 0.0;

    double TAT = 0.0;
    double overhead = 0.0;

    size_t count = listCount(info->hProcs);
    void * hProc = NULL;
    for (size_t i = 0; i < count; i++) {
        hProc = listGet(info->hProcs, i);

        TAT =  (double)procTAT(hProc);
        overhead = TAT / (double)procExecTime(hProc);

        sumTAT += TAT;
        sumOverhead += overhead;
        if (overhead > maxOverhead)
            maxOverhead = overhead;
    }

    // display results
    printf("Turnaround time %g\n", ceil(sumTAT / (double)count));
    printf("Time overhead %g %g\n", roundf(maxOverhead * 100.0) / 100.0, roundf(sumOverhead * 100.0 / (double)count) / 100.0);
    printf("Makespan %d\n", time);
}

// helper function definitions
// Loads processes from the file defined in cpuInfo
// Returns the pointer to the null-terminated array of processes if successful.
// Returns NULL otherwise
static void loadProcesses(CPUINFO * cpuInfo) {
    INFON(cpuInfo)

    info->hProcs = listCreate();

    if (!info->hProcs) {
        return;
    }

    FILE * hFile = fopen(cpuInfo->processListFile, "r");

    if (!hFile) {
        listDelete(info->hProcs);
        info->hProcs = NULL;
        return;
    }

    char sBuffer[80];
    unsigned int arrive, pid, exec;
    void * hProc = NULL;

    while (fscanf(hFile, "%u %u %u %s\n", &arrive, &pid, &exec, sBuffer) != EOF) {
        hProc = procCreate(arrive, pid, exec, strcmp(sBuffer, "p") == 0, info->processors);

        if (!listPush(info->hProcs, hProc)) {
            listDelete(info->hProcs);
            procDelete(hProc);
            info->hProcs = NULL;
            return;
        }
    }

    fclose(hFile); // close the file after reading
}

