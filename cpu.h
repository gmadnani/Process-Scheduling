#ifndef CPU_H_
#define CPU_H_

// Include dependencies
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"

// initialize a cpu with the arguments
void * cpuInit(int argc, char** argv);

// run a single frame in the cpu
int cpuRun(void * cpuHandle, unsigned int time);

// delete the cpu
void cpuDelete(void * cpuHandle);

// compute simulation statistics
void cpuStats(void * cpuHandle, unsigned int time);

// schedule the process to the cores
size_t cpuSchedule(void * cpuHandle, unsigned int time);

// schedule the process to the cores using my own algorithm
size_t cpuOwnSchedule(void * cpuHandle, unsigned int time);

// run a core
// returns the number of unfinished processes in the queue
int cpuRunCore(void * hCore, void * hIterator, int time, void * hFinishedTargets, void ** hStartingTargets, void ** lastTarget); 

// PROCESSOR functions

// Creates a processor
// Returns the pointer on success
// Returns NULL if failed
void * processorCreate(unsigned cpuID); 

// Destroys a processor
void processorDelete(void * hProcessor);

// Performs a single step of the processor
void processorRun(void * hProcessor, unsigned int time, void * hRunningSubProcs);

// Returns the list of pending processes
void * processorPending(void * hProcessor);

// Calculates the total remaining time of the pending processes
unsigned int processorRemainingTime(void * hProcessor);

// Gets the nearest deadline of the processor
unsigned int processorDeadline(void * hProcessor);

// Get the target
void * processorCurrentSubProc(void * hProcessor);

// Clear current sub proc
void processorClearCurrent(void * hProcessor);

// Get processor id
unsigned int processorID(void * hProcessor);

#endif