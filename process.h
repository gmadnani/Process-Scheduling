#ifndef PROCESS_H_
#define PROCESS_H_

// Include dependencies
#include <math.h>
#include "list.h"

// Defines all the procedures used to handle processes and sub process objects

// PROCESS FUNCTIONS

// Creates a handler to a process
// Returns handler to the process
// Returns NULL if failed
void * procCreate(unsigned int arrivalTime, 
                  unsigned int pid, unsigned int execTime, 
                  unsigned int canParallel, 
                  int processors);



// Deletes a handler to a process
void procDelete(void * hProcess);

// Get remaining execution tim
// returns the remaining time to complete
unsigned int procRem(void * hProcess);

// Get the arrival time of process
unsigned int procArrivalTime(void * hProcess);

// Get pid of process
unsigned int procID(void * hProcess);

// Get turnaround time
unsigned int procTAT(void * hProcess);

// Get exec time
unsigned int procExecTime(void * hProcess);

// Get number of sub processes
unsigned int procSubs(void * hProcess);

// Get handle of the subprocesses
void * procSubsHandle(void * hProcess);

// SUBPROCESS FUNCTIONS

// Creates a handler to a subprocess
// Returns the pointer to the subprocess
// Returns NULL if failed
void * subProcCreate(unsigned int pid, unsigned int execTime, void * hProcess);

// Deletes the handler of a sub process
void subProcDelete(void * hSubProc);

// Runs a single step of a sub process
// Returns 1 if finished
// Returns 0 otherwise
unsigned int subProcExec(void * hSubProc, unsigned int timeFrame);

// Execution time
unsigned int subProcExecTime(void * hSubProc);

// Completion time
unsigned int subProcCompletion(void * hSubProc);

// Parent process
void * subProcParent(void * hSubProc);

// Get worked 
unsigned int subProcWorked(void * hSubProc);

// Get id
unsigned int subProcID(void * hSubProc);

// Get remaining time
unsigned int subProcRem(void * hSubProc);

// Get waiting time of the sub process
unsigned int subProcWaiting(void * hSubProc, unsigned int time);

#endif