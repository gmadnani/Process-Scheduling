#include "cpu.h"

// Define struct for the processor
typedef struct {
    unsigned int cpuID;
    void * hListPending; // list of pending sub processes assigned to this processor
    void * hCurrentSubProc; // the current sub process being executed
} PROCESSOR;

// PROCESSOR FUNCTIONS
#define PCR(h) if (!h) { return 0; } PROCESSOR* pcr = (PROCESSOR*)h;
#define PCRN(h) if (!h) { return; } PROCESSOR* pcr = (PROCESSOR*)h;

// Creates a processor
// Returns the pointer on success
// Returns NULL if failed
void * processorCreate(unsigned int cpuID) {
    PROCESSOR * p = (PROCESSOR*)calloc(1, sizeof(PROCESSOR));

    if (!p) {
        return NULL;
    }

    p->cpuID = cpuID;
    p->hListPending = listCreate();

    if (!p->hListPending) {
        processorDelete(p);
        return NULL;
    }

    p->hCurrentSubProc = NULL;

    return p;
}

// Destroys a processor
void processorDelete(void * hProcessor) {
    PCRN(hProcessor)

    listDelete(pcr->hListPending);
    free(pcr);
}

// Performs a single step of the processor
void processorRun(void * hProcessor, unsigned int timeFrame, void * hRunningSubProcs) {
    PCRN(hProcessor)

    // Check if current sub process is finished
    if (pcr->hCurrentSubProc) {
        if (subProcRem(pcr->hCurrentSubProc) == 0) {
            pcr->hCurrentSubProc = NULL; // clear current process
        }
    } 

    if (pcr->hCurrentSubProc) {
        // run current sub process
        subProcExec(pcr->hCurrentSubProc, timeFrame);
    } else {
        // FIFO - get the first pending sub process
        size_t count = listCount(pcr->hListPending);
        
        if (count > 0) {       
            pcr->hCurrentSubProc = listRemove(pcr->hListPending, 0); // remove from pending list
            subProcExec(pcr->hCurrentSubProc, timeFrame); // execute new sub process
            listSet(hRunningSubProcs, pcr->cpuID, pcr->hCurrentSubProc);
        } 
    }
}

// Returns the list of pending processes
void * processorPending(void * hProcessor) {
    PCR(hProcessor);

    return pcr->hListPending;
}

// Calculates the total remaining time of the pending sub processes
unsigned int processorRemainingTime(void * hProcessor) {
    PCR(hProcessor)

    int remTime = 0;

    if (pcr->hCurrentSubProc) {
        remTime += subProcRem(pcr->hCurrentSubProc);
    }

    size_t count = listCount(pcr->hListPending);

    for (size_t i = 0; i < count; i++) {
        remTime += subProcRem(listGet(pcr->hListPending, i));
    }

    return remTime;
}

void * processorCurrentSubProc(void * hProcessor) {
    PCR(hProcessor)

    return pcr->hCurrentSubProc;
}

unsigned int processorID(void * hProcessor) {
    PCR(hProcessor)

    return pcr->cpuID;
}

// Clear current sub proc
void processorClearCurrent(void * hProcessor) {
    PCRN(hProcessor)

    pcr->hCurrentSubProc = NULL;
}

// Gets the nearest deadline of the processor
unsigned int processorDeadline(void * hProcessor) {
    PCR(hProcessor);

    unsigned int nearestDeadline = 0, found = 0;

    if (pcr->hCurrentSubProc) {
        nearestDeadline = procArrivalTime(subProcParent(pcr->hCurrentSubProc)) + subProcExecTime(pcr->hCurrentSubProc);
        found = 1;
    }

    size_t count = listCount(pcr->hListPending);
    for (size_t i = 0; i < count; i++) {
        void * hSubProc = listGet(pcr->hListPending, i);
        unsigned deadline = procArrivalTime(subProcParent(hSubProc)) + subProcExecTime(hSubProc);
        if (found) {
            if (deadline > nearestDeadline) { // the deadline of the processor is the longest one
                nearestDeadline = deadline;
            }
        } else {
            nearestDeadline = deadline;
            found = 1;
        }
    }

    return nearestDeadline;
}