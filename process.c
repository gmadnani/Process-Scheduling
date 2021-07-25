#include "process.h"


// STRUCT DEFINITIONS
typedef struct {
    unsigned int arrival;
    unsigned int exec;
    unsigned int pid;
    void * hSubProcesses; // list of sub process
} PROCESS;

typedef struct {
    unsigned int exec;
    unsigned int pid;
    unsigned int completion;
    unsigned int worked;
    void * hProcess; // handle of the parent process
} SUBPROCESS;

// PROCESS DEFINITIONS
#define PROC(h) if (!h) { return 0; } PROCESS* proc = (PROCESS*)h;
#define PROCN(h) if (!h) { return; } PROCESS* proc = (PROCESS*)h;

// Creates a handler to a process
void * procCreate(unsigned int arrivalTime, unsigned int pid, unsigned int execTime, unsigned int canParallel, int processors) {
    PROCESS* proc = (PROCESS*)calloc(1, sizeof(PROCESS));

    if (!proc) {
        return NULL;
    }

    // Set variables
    proc->arrival = arrivalTime;
    proc->pid = pid;
    proc->exec = execTime;
    proc->hSubProcesses = listCreate();

    if (!proc->hSubProcesses) {
        free(proc);
        return NULL;
    }

    void * hSubProc = NULL;
    if (canParallel) {
        unsigned int k = 0; // number of subProcesses
        unsigned int subProcExecTime = 1; // for synchronization

        // compute the execution time 
        if ((unsigned int)processors <= execTime) {
            k = processors;
            subProcExecTime += (unsigned int)ceil((double)execTime / (double)processors);
        } else {
            k = execTime;
            subProcExecTime += 1;
        }

        // create sub processes       
        for (size_t i = 0; i < k; i++) {
            hSubProc = subProcCreate(i, subProcExecTime, proc);

            if (!hSubProc) { // failed to create sub process
                procDelete(proc);
                return NULL;
            }

            if (!listPush(proc->hSubProcesses, hSubProc)) { // failed to insert sub process to the list
                procDelete(proc);
                return NULL;
            }
        }
    } else {
        hSubProc = subProcCreate(0, execTime, proc);

        if (!listPush(proc->hSubProcesses, hSubProc)) { // failed to insert sub process to the list
            procDelete(proc);
            return NULL;
        }
    }  

    return proc;
}

// Deletes a handler to a process
void procDelete(void * hProcess) {
    PROCN(hProcess)
        
    // delete each sub process
    size_t count = listCount(proc->hSubProcesses);
    for (size_t i = 0; i < count; i++) {
        subProcDelete(listGet(proc->hSubProcesses, i));
    }
    
    listDelete(proc->hSubProcesses);
    free(proc);
}


// Get remaining execution tim
// returns the remaining time to complete
unsigned int procRem(void * hProcess) {
    PROC(hProcess)

    size_t count = listCount(proc->hSubProcesses);
    unsigned int rem = 0;

    for (size_t i = 0; i < count; i++) {
        rem += subProcRem(listGet(proc->hSubProcesses, i));
    }

    return rem;
}

// Gets the arrival time of proc
unsigned int procArrivalTime(void * hProcess) {
    PROC(hProcess)

    return proc->arrival;
}

// Get pid of process
unsigned int procID(void * hProcess) {
    PROC(hProcess)

    return proc->pid;
}

// Get turnaround time
unsigned int procTAT(void * hProcess) {
    PROC(hProcess)

    size_t count = listCount(proc->hSubProcesses);

    unsigned int completion = 0, subCompletion = 0;

    for (size_t i = 0; i < count; i++) {
        subCompletion = subProcCompletion(listGet(proc->hSubProcesses, i));

        if (subCompletion > completion) {
            completion = subCompletion;
        }
    }

    return completion - proc->arrival + 1;
}

// Get exec time
unsigned int procExecTime(void * hProcess) {
    PROC(hProcess)

    return proc->exec;
}

// Get number of sub processes
unsigned int procSubs(void * hProcess) {
    PROC(hProcess)

    return listCount(proc->hSubProcesses);
}


// Get handle of the subprocesses
void * procSubsHandle(void * hProcess) {
    PROC(hProcess)

    return proc->hSubProcesses;
}

// SUBPROCESS DEFINITIONS

#define SUBPROC(hSubProc) if (!hSubProc) { return 0; } SUBPROCESS* subProc = (SUBPROCESS*)hSubProc;
#define SUBPROCN(hSubProc) if (!hSubProc) { return; } SUBPROCESS* subProc = (SUBPROCESS*)hSubProc;

// Creates a handler to a subprocess
// Returns the pointer to the subprocess
// Returns NULL if failed
void * subProcCreate(unsigned int pid, unsigned int execTime, void * hProcess) {
    if (!hProcess) { // there should be a parent process to the sub process
        return NULL;
    }

    SUBPROCESS* subProc = (SUBPROCESS*)calloc(1, sizeof(SUBPROCESS));

    if (!subProc) {
        return NULL;
    }

    subProc->completion = 0;
    subProc->exec = execTime;
    subProc->hProcess = hProcess;
    subProc->pid = pid; // sub process id
    subProc->worked = 0;
                    
    return subProc;
}


// Deletes the handler of a sub process
void subProcDelete(void * hSubProc) {
    SUBPROCN(hSubProc);

    free(subProc);
}

// Completion time
unsigned int subProcCompletion(void * hSubProc) {
    SUBPROC(hSubProc)

    return subProc->completion;
}

// Execution time
unsigned int subProcExecTime(void * hSubProc) {
    SUBPROC(hSubProc)

    return subProc->exec;
}

// Parent process
void * subProcParent(void * hSubProc) {
    SUBPROC(hSubProc)

    return subProc->hProcess;
}

// Get worked 
unsigned int subProcWorked(void * hSubProc) {
    SUBPROC(hSubProc)

    return subProc->worked;
}

// Runs a single step of a sub process
// Returns 1 if finished
// Returns 0 otherwise
unsigned int subProcExec(void * hSubProc, unsigned int timeFrame) {
    SUBPROC(hSubProc)

    if (subProc->worked < subProc->exec) {
        subProc->worked++;

        if (subProc->worked == subProc->exec) {
            subProc->completion = timeFrame;
        }
        return 0;
    } else {
        return 1;
    }
}

// Get id
unsigned int subProcID(void * hSubProc) {
    SUBPROC(hSubProc)

    return subProc->pid;
}

// Get remaining time
unsigned int subProcRem(void * hSubProc) {
    SUBPROC(hSubProc)

    return subProc->exec - subProc->worked;
}

// Get waiting time of the sub process
unsigned int subProcWaiting(void * hSubProc, unsigned int time) {
    SUBPROC(hSubProc)

    return time - procArrivalTime(subProcParent(subProc->hProcess)) - subProc->worked;
}